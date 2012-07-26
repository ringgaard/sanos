//
// impdef.c
//
// Utility for generating import definitions for DLLs
//
// Copyright (C) 2011 Michael Ringgaard. All rights reserved.
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <os/pe.h>

void *get_rva(char *dll, struct image_header *imghdr, unsigned long rva) {
  int i;

  // Find section containing the relative virtual address
  for (i = 0; i < imghdr->header.number_of_sections; i++) {
    struct image_section_header *sect = &imghdr->sections[i];
    if (sect->virtual_address <= rva && sect->virtual_address + sect->size_of_raw_data > rva) {
      return dll + sect->pointer_to_raw_data + (rva - sect->virtual_address);
    }
  }
  return NULL;
}

void *get_image_directory(char *dll, struct image_header *imghdr, int dir) {
  return get_rva(dll, imghdr, imghdr->optional.data_directory[dir].virtual_address);
}

int main(int argc, char *argv[]) {
  char *dll_filename;
  char *def_filename;
  char *dll;
  int fd;
  unsigned int size, i;
  struct stat st;
  struct dos_header *doshdr;
  struct image_header *imghdr;
  struct image_export_directory *expdir;
  unsigned long *names;
  char *module_name;
  FILE *output;

  // Parse command line
  if (argc == 2) {
    dll_filename = argv[1];
    def_filename = NULL;
  } else if (argc == 3) { 
    dll_filename = argv[1];
    def_filename = argv[2];
  } else { 
    fprintf(stderr, "usage: impdef <dll> [<def>]\n");
    return 1;
  }
  
  // Load DLL image into memory
  fd = open(dll_filename, O_RDONLY | O_BINARY);
  if (fd < 0 || fstat(fd, &st) < 0) {
    perror(dll_filename);
    return 1;
  }
  size = st.st_size;
  dll = (char *) malloc(size);
  if (read(fd, dll, size) != size) {
    perror(dll_filename);
    free(dll);
    return 1;
  }
  close(fd);

  // Check PE file signature
  doshdr = (struct dos_header *) dll;
  imghdr = (struct image_header *) (dll + doshdr->e_lfanew);
  if (doshdr->e_lfanew > size || imghdr->signature != IMAGE_PE_SIGNATURE) {
    fprintf(stderr, "%s: Not a PE file\n", dll_filename);
    free(dll);
    return 1;
  }
  
  // Open output file.
  if (def_filename) {
    output = fopen(def_filename, "wb");
    if (output == NULL) {
      perror(def_filename);
      free(dll);
      return 1;
    }
  } else {
    output = stdout;
  }
  
  // Output exported functions.
  expdir = get_image_directory(dll, imghdr, IMAGE_DIRECTORY_ENTRY_EXPORT);
  module_name = get_rva(dll, imghdr, expdir->name);
  fprintf(output, "LIBRARY %s\r\n\r\nEXPORTS\r\n", module_name);
  names = get_rva(dll, imghdr, expdir->address_of_names);
  for (i = 0; i < expdir->number_of_names; i++) {
    char *name = get_rva(dll, imghdr,  names[i]);
    fprintf(output, "%s\r\n", name);
  }

  if (def_filename) fclose(output);
  free(dll);  
  return 0;
}
