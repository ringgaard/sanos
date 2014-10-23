#include <os.h>
#include <stdio.h>
#include <stdlib.h>

void step(int n) {
  printf(" %d", n);
  fflush(stdout);
  msleep(100);
}

int copy_file(char *srcfn, char *dstfn) {
  static char block[64 * 1024];
  int fin;
  int fout;
  int bytes;
  int rc;

  step(4);
  fin = open(srcfn, O_BINARY);
  if (fin < 0) return fin;

  step(5);
  fout = open(dstfn,  O_CREAT | O_EXCL | O_BINARY, S_IREAD | S_IWRITE);
  if (fout < 0) return fout;

  step(6);
  while ((bytes = read(fin , block, sizeof block)) > 0) {
    rc = write(fout, block, bytes);
    if (rc < 0) return rc;
  }

  step(7);
  if (bytes < 0) return bytes;

  step(8);
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
  step(1);
  mkfs("rd0", "dfs", "quick");

  // Mount ramdisk on /var
  step(2);
  mount("dfs", "/var", "rd0", NULL);

  // Copy UNIX disk image to ramdisk
  step(3);
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
    printf("%c\n", cmd);

    switch (cmd) {
      case 'b':
      case 'B':
        // Start PDP-11 simulator
        printf("TRYING UNIT DL0\n");
        msleep(1000);
        printf("\nBOOTING FROM DL0\n");        
        spawnl(P_WAIT, "pdp11.exe", "pdp11", "/unix/unixv7", NULL);
        break;

      case 'd':
      case 'D':
        // Start simulator witout Unix
        printf("");
        spawnl(P_WAIT, "pdp11.exe", "pdp11", NULL);
        break;

      case 'x':
       printf("\033c\f");
       return 0;

      case 'l':
      case 'L':
        // List boot devices
        printf("\n");
        printf("Bootable devices\n");
        printf("Device           Max\n");
        printf("Name Type       Units\n");
        printf("\n");
        printf(" DL  RL01/02     4\n");
        break;

      case 'h':
      case 'H':
      case '?':
        // Display help message
        printf("\nWelcome to PDP-11 7th Edition UNIX\n\n");
        printf("  1) Select Boot from menu by typing b\n");
        printf("  2) At the @ prompt type: boot<enter>\n");
        printf("  3) At the : prompt type: rl(0,0)rl2unix<enter>\n");
        printf("  4) At the # prompt type <ctrl-d> and login as root with password root\n");
        break;
    }
  }

  return 0;
}
