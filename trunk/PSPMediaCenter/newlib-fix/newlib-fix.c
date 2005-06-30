#include <stdlib.h>
#include <pspiofilemgr.h>
#include <pspkernel.h>

int open(const char *fname, int flags, ...)
{
    return sceIoOpen(fname, flags, 0777);
}
int close(int handle)
{
    return sceIoClose(handle);
}

ssize_t read(int handle, void *buffer, size_t size)
{
    return sceIoRead(handle, buffer, size);
}

ssize_t write(int handle, const void *buffer, size_t size)
{
    return sceIoWrite(handle, buffer, size);
}

off_t lseek(int handle, off_t position, int wheel)
{
    return sceIoLseek(handle, position, wheel);
}

off_t tell(int handle)
{
    return sceIoLseek(handle, 0, 1);
}
int kill(pid_t pid, int sig)
{
    return -1;
}
int isatty(int desc)
{
    return 0;
}

pid_t getpid()
{
    pid_t pid = 0;
    return pid;
}

int fstat(int fd, io_stat_t * buf)
{
/*
struct stat {
                  dev_t         st_dev;      // device 
                  ino_t         st_ino;      // inode 
                  mode_t        st_mode;     // protection 
                  nlink_t       st_nlink;    // number of hard links 
                  uid_t         st_uid;      // user ID of owner 
                  gid_t         st_gid;      // group ID of owner 
                  dev_t         st_rdev;     // device type (if inode device) 
                  off_t         st_size;     // total size, in bytes 
                  blksize_t     st_blksize;  // blocksize for filesystem I/O 
                  blkcnt_t      st_blocks;   // number of blocks allocated 
                  time_t        st_atime;    // time of last access 
                  time_t        st_mtime;    // time of last modification 
                  time_t        st_ctime;    // time of last status change 
              };*/

    return 0;
}
