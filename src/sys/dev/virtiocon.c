//
// virtiocon.c
//
// Console driver for virtio
//
// Copyright (C) 2012 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.  
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.  
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
// SUCH DAMAGE.
// 

#include <os/krnl.h>

//
// Feature bits
//

#define VIRTIO_CON_F_SIZE      0  // Does host provide console size?
#define VIRTIO_CON_F_MULTIPORT 1  // Does host provide multiple ports?

//
// Virtual console configuration
//

struct virtio_console_config {
  unsigned short cols;            // Number of screen columns
  unsigned short rows;            // Number of screen rows
  unsigned long max_ports;        // Maximum number of ports for device
};

//
// Virtual console device data
//

struct virtiocon {
  struct virtio_device vd;
  struct virtio_console_config config; 
  struct virtio_queue input_queue;
  struct virtio_queue output_queue;
  dev_t devno;
};

static int virtiocon_ioctl(struct dev *dev, int cmd, void *args, size_t size) {
  return -ENOSYS;
}

static int virtiocon_read(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  return -ENOSYS;
}

static int virtiocon_write(struct dev *dev, void *buffer, size_t count, blkno_t blkno, int flags) {
  struct virtiocon *vcon = (struct virtiocon *) dev->privdata;
  struct scatterlist sg[1];
  int rc;

  // Issue request
  sg[0].data = buffer;
  sg[0].size = count;
  rc = virtio_enqueue(&vcon->output_queue, sg, 1, 0, self());
  if (rc < 0) return rc;
  virtio_kick(&vcon->output_queue);
  
  // Wait for request to complete
  enter_wait(THREAD_WAIT_DEVIO);

  return count;
}

static int virtiocon_input_callback(struct virtio_queue *vq) {
  char *buf;
  unsigned int len;
  int i;

  //kprintf("[VCI]");
  while ((buf = virtio_dequeue(vq, &len)) != NULL) {
    //kprintf("[vcinp%d:", len);
    //for (i = 0; i < len; ++i) kprintf("%c", buf[i]);
    //kprintf("]");
  }

  return 0;
}

static int virtiocon_output_callback(struct virtio_queue *vq) {
  struct thread *thread;
  unsigned int len;

  kprintf("[VCO]");
  while ((thread = virtio_dequeue(vq, &len)) != NULL) {
    mark_thread_ready(thread, 1, 2);
  }
  
  return 0;
}

struct driver virtiocon_driver = {
  "virtiocon",
  DEV_TYPE_STREAM,
  virtiocon_ioctl,
  virtiocon_read,
  virtiocon_write
};

static int install_virtiocon(struct unit *unit) {
  struct virtiocon *vcon;
  int rc;
  int size;
  int i;

  // Setup unit information
  if (!unit) return -ENOSYS;
  unit->vendorname = "VIRTIO";
  unit->productname = "VIRTIO Virtual Console Device";

  // Allocate memory for device
  vcon = kmalloc(sizeof(struct virtiocon));
  if (vcon == NULL) return -ENOMEM;
  memset(vcon, 0, sizeof(struct virtiocon));

  // Initialize virtual device
  rc = virtio_device_init(&vcon->vd, unit, VIRTIO_CON_F_SIZE);
  if (rc < 0) return rc;

  // Get console device configuration
  virtio_get_config(&vcon->vd, &vcon->config, sizeof(vcon->config));

  // Initialize queues for console
  rc = virtio_queue_init(&vcon->input_queue, &vcon->vd, 0, virtiocon_input_callback);
  if (rc < 0) return rc;
  rc = virtio_queue_init(&vcon->output_queue, &vcon->vd, 1, virtiocon_output_callback);
  if (rc < 0) return rc;

  // Fill input queue
  size = virtio_queue_size(&vcon->input_queue);
  for (i = 0; i < size; ++i) {
    struct scatterlist sg[1];
    char *data = kmalloc(PAGESIZE);
    if (!data) return -ENOMEM;
    sg[0].data = data;
    sg[0].size = PAGESIZE;
    virtio_enqueue(&vcon->input_queue, sg, 0, 1, data);
  }
  virtio_kick(&vcon->input_queue);

  // Create device
  vcon->devno = dev_make("vc#", &virtiocon_driver, unit, vcon);
  virtio_setup_complete(&vcon->vd, 1);
  kprintf(KERN_INFO "%s: virtio console, %dx%d, %d ports, feats=%d\n", 
          device(vcon->devno)->name, 
          vcon->config.cols, vcon->config.rows, vcon->config.max_ports, vcon->vd.features);

  return 0;
}

int __declspec(dllexport) virtiocon(struct unit *unit, char *opts) {
  return install_virtiocon(unit);
}

