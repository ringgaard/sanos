#ifndef TYPES_H
#define TYPES_H

#define VFS_S_IFMT           0170000         // File type mask
#define VFS_S_IFREG        0100000         // Regular file
#define VFS_S_IFDIR        0040000         // Directory

#define VFS_S_IREAD        0000400         // Read permission, owner
#define VFS_S_IWRITE       0000200         // Write permission, owner
#define VFS_S_IEXEC        0000100         // Execute/search permission, owner

#define VFS_S_ISREG(m)      (((m) & VFS_S_IFMT) == VFS_S_IFREG)
#define VFS_S_ISDIR(m)      (((m) & VFS_S_IFMT) == VFS_S_IFDIR)

#define VFS_S_IRWXU 00700
#define VFS_S_IRWXG 00070
#define VFS_S_IRWXO 00007

typedef unsigned int vfs_devno_t;
typedef unsigned int vfs_blkno_t;
typedef unsigned int vfs_ino_t;
typedef unsigned int vfs_loff_t;
typedef unsigned int vfs_time_t;

#ifdef WIN32
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#endif

#ifdef __linux__
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#endif

#endif

