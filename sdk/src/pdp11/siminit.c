#include <os.h>
#include <stdio.h>
#include <stdlib.h>

int copy_file(char *srcfn, char *dstfn) {
  static char block[64 * 1024];
  int fin;
  int fout;
  int bytes;
  int rc;

  printf(" 4");
  fin = open(srcfn, O_BINARY);
  if (fin < 0) return fin;

  printf(" 5");
  fout = open(dstfn,  O_CREAT | O_EXCL | O_BINARY, S_IREAD | S_IWRITE);
  if (fout < 0) return fout;

  printf(" 6");
  while ((bytes = read(fin , block, sizeof block)) > 0) {
    rc = write(fout, block, bytes);
    if (rc < 0) return rc;
  }

  printf(" 7");
  if (bytes < 0) return bytes;

  printf(" 8");
  close(fin);
  close(fout);

  return 0;
}

int main(int argc, char *argv[]) {
  printf("\f\033[32m"
         "KDF11B-BH ROM V0.9\n"
         "\n"
         "  512KB MEMORY\n"
         "9 STEP MEMORY TEST\n"
         "  STEP");

  // Create DFS file system on ramdisk
  printf(" 1");
  mkfs("rd0", "dfs", "quick");

  // Mount ramdisk on /var
  printf(" 2");
  mount("dfs", "/var", "rd0", NULL);

  // Copy UNIX disk image to ramdisk
  printf(" 3");
  copy_file("/unix/unix_v7_rl.dsk", "/var/unix_v7_rl.dsk");
  printf(" 9\n");

  // Done initializing
  printf("TOTAL MEMORY ERRORS =     0\n"
         "CLOCK ENABLED\n");

  // Boot menu
  for (;;) {
    int cmd;

    printf("\nType ? for HELP\n");
    printf("Enter one of [Boot, Diagnose, Help, List, Map]:");
    cmd = getchar();
    switch (cmd) {
      case 'b':
      case 'B':
        // Start PDP-11 simulator
        printf("\nTRYING UNIT DL0\n");
        msleep(1000);
        printf("\nBOOTING FROM DL0\n");        
        system("pdp11 /unix/unixv7");
        break;

      case 'd':
      case 'D':
        // Start simulator witout Unix
        printf("\n");
        system("pdp11");
        break;

      case 'x':
       printf("\033c\f");
       return 0;

      case 'h':
      case 'H':
      case '?':
        // Display help message
        printf("\n\nWelcome to PDP-11 7th Edition UNIX\n\n");
        printf("  1) Select Boot from menu by typing b\n");
        printf("  2) At the @ prompt type: boot<enter>\n");
        printf("  3) At the : prompt type: rl(0,0)rl2unix<enter>\n");
        printf("  4) At the # prompt type <ctrl-d> and login as root with password root\n");
        printf("\n");
        break;
    }
  }

  return 0;
}
