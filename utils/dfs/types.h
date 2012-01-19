#ifndef TYPES_H
#define TYPES_H

#define S_IFMT         0170000         // File type mask
#define S_IFSOCK       0140000         // Socket
#define S_IFLNK        0120000         // Symbolic link
#define S_IFREG        0100000         // Regular file
#define S_IFBLK        0060000         // Block device
#define S_IFDIR        0040000         // Directory
#define S_IFCHR        0020000         // Character device
#define S_IFIFO        0010000         // Pipe

#define S_IREAD        0000400         // Read permission, owner
#define S_IWRITE       0000200         // Write permission, owner
#define S_IEXEC        0000100         // Execute/search permission, owner

#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

typedef unsigned int devno_t;
typedef unsigned int blkno_t;
typedef unsigned int ino_t;
typedef unsigned int loff_t;

#endif
