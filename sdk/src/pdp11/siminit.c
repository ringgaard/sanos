#include <os.h>
#include <stdio.h>
#include <stdlib.h>

int copy_file(char *srcfn, char *dstfn)
{
  static char block[64 * 1024];
  int fin;
  int fout;
  int bytes;
  int rc;

  fin = open(srcfn, O_BINARY);
  if (fin < 0) return fin;

  fout = open(dstfn,  O_CREAT | O_EXCL | O_BINARY, S_IREAD | S_IWRITE);
  if (fout < 0) return fout;

  while ((bytes = read(fin , block, sizeof block)) > 0)
  {
    rc = write(fout, block, bytes);
    if (rc < 0) return rc;
  }

  if (bytes < 0) return bytes;

  close(fin);
  close(fout);

  return 0;
}

int main(int argc, char *argv[])
{
  printf("\r\033[K");
  printf("Initializing...");

  // Create DFS file system on ramdisk
  mkfs("rd0", "dfs", "quick");

  // Mount ramdisk on /var
  mount("dfs", "/var", "rd0", NULL);

  // Copy UNIX disk image to ramdisk
  copy_file("/unix/unix_v7_rl.dsk", "/var/unix_v7_rl.dsk");

  // Start PDP-11 simulator
  printf("\r                                  \r");
  printf("Welcome to PDP-11 7th Edition UNIX\n\n");
  printf("  1) At the @ prompt type: boot<enter>\n");
  printf("  2) At the : prompt type: rl(0,0)rl2unix<enter>\n");
  printf("  3) At the # prompt type <ctrl-d> and login as root with password root\n");
  printf("\n");

  system("pdp11 /unix/unixv7");

  return 0;
}
