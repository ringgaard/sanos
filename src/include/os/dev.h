//
// dev.h
//
// Copyright (c) 2001 Michael Ringgaard. All rights reserved.
//
// Device Manager
//

#ifndef DEV_H
#define DEV_H

struct dev;
struct pci_dev;
struct pnp_dev;

#define NODEV (-1)

#define DEVNAMELEN              32
#define MAX_DEVICES             64
#define MAX_RESOURCES           32
#define MAX_DEVS                64

#define DEV_TYPE_STREAM		1
#define DEV_TYPE_BLOCK		2
#define DEV_TYPE_PACKET		3

#define IOCTL_GETBLKSIZE        1
#define IOCTL_GETDEVSIZE        2
#define IOCTL_GETGEOMETRY       3

#define DEVICE_TYPE_LEGACY      0
#define DEVICE_TYPE_PCI         1
#define DEVICE_TYPE_PNP         2

#define RESOURCE_UNUSED	        0
#define RESOURCE_IO	        1
#define RESOURCE_MEM	        2
#define RESOURCE_IRQ	        3
#define RESOURCE_DMA	        4

#define BIND_PCI_CLASS          'C'
#define BIND_PCI_DEVICE         'D'
#define BIND_PNP_TYPECODE       'T'
#define BIND_PNP_EISAID         'E'

struct resource
{
  int type;
  int flags;
  unsigned long start;
  unsigned long len;
};

struct driver
{
  char *name;
  int type;

  int (*ioctl)(struct dev *dev, int cmd, void *args, size_t size);
  int (*read)(struct dev *dev, void *buffer, size_t count, blkno_t blkno);
  int (*write)(struct dev *dev, void *buffer, size_t count, blkno_t blkno);

  int (*attach)(struct dev *dev, struct eth_addr *hwaddr);
  int (*detach)(struct dev *dev);
  int (*transmit)(struct dev *dev, struct pbuf *p);
};

struct dev 
{
  char name[DEVNAMELEN];
  struct driver *driver;
  struct device *device;
  void *privdata;
  int refcnt;

  struct netif *netif;
  int (*receive)(struct netif *netif, struct pbuf *p);
};

struct device
{
  int type;
  char *name;
  struct dev *dev;
  
  unsigned long classcode;
  unsigned long devicecode;

  int numres;
  struct resource res[MAX_RESOURCES];
  
  union
  {
    struct pci_dev *pci;
    struct pnp_dev *pnp;
  };
};

struct binding
{
  int type;
  unsigned long code;
  unsigned long mask;
  char *module;
};

struct geometry
{
  int cyls;
  int heads;
  int spt;
  int sectorsize;
  int sectors;
};

extern struct dev *devtab[];
extern unsigned int num_devs;

extern struct device *devicetab[];
extern int num_devices;

void install_drivers();

krnlapi struct device *register_device(int type, unsigned long classcode, unsigned long devicecode);
krnlapi int add_resource(struct device *dv, int type, int flags, unsigned long start, unsigned long len);

krnlapi struct dev *device(devno_t devno);

krnlapi devno_t dev_make(char *name, struct driver *driver, struct device *dv, void *privdata);
krnlapi devno_t dev_open(char *name);
krnlapi int dev_close(devno_t devno);

krnlapi int dev_ioctl(devno_t devno, int cmd, void *args, size_t size);
krnlapi int dev_read(devno_t devno, void *buffer, size_t count, blkno_t blkno);
krnlapi int dev_write(devno_t devno, void *buffer, size_t count, blkno_t blkno);

krnlapi int dev_attach(devno_t devno, struct netif *netif, int (*receive)(struct netif *netif, struct pbuf *p));
krnlapi int dev_detach(devno_t devno);
krnlapi int dev_transmit(devno_t devno, struct pbuf *p);
krnlapi int dev_receive(devno_t devno, struct pbuf *p);

#endif
