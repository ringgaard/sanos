#ifndef GETOPT_H
#define GETOPT_H

extern int opterr;
extern int optind;
extern int optopt;
extern char *optarg;

int getopt(int argc, char **argv, char *opts);

#endif
