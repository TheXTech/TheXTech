#ifdef VITA
#include "dirman_vita.h"
#include <Logger/logger.h>

// #include <sys/_default_fcntl.h>
#include <sys/stat.h>


int dirfd(DIR_ *dirp)
{
    if(dirp == nullptr) return;

    SceUID fp = 
        ((*((char*)dirp) + 0)) >> 0 | 
        ((*((char*)dirp) + 1)) >> 8 | 
        ((*((char*)dirp) + 2)) >> 16 | 
        ((*((char*)dirp) + 3)) >> 24;
        // Get the file descriptor of a POSIX directory struct.
        // This will somehow have to become an SceFd 
        // pLogDebug("DIR STAT GIVEN dirp of %p. We're returning dirp->uid %d", (void*)dirp, fp);
    if(fp == 0) return -1;
    return fp;
}



extern "C" {
    int mkdir(const char *path, mode_t mode)
    {
        return sceIoMkdir(path, mode);
    }

    int rmdir(const char *__path)
    {
        return sceIoRmdir(__path);
    }

    int dup2(int oldfd, int newfd)
    {
        return oldfd;
    }

    /*
    The fstatat() function shall be equivalent to the stat() or lstat() function, depending on the value of flag (see below), 
     except in the case where path specifies a relative path. 
    In this case the status shall be retrieved from a file relative to the directory 
     associated with the file descriptor fd instead of the current working directory. 
    If the access mode of the open file description associated with the file descriptor 
     is not O_SEARCH, the function shall check whether directory searches are permitted using the current 
    permissions of the directory underlying the file descriptor. 
    
    If the access mode is O_SEARCH, the function shall not perform the check.
    */
   
    int fstatat(int fd, const char* __restrict path,
       struct stat* __restrict buf, int flag)
    {
        // SceIoDirent _direntFd = 
        SceIoStat _temp;

        int ret = 0;
        if(fd >= 0)
        {
            // ret = sceIoGetstatByFd(fd, &_temp);
            ret = fstat(fd, buf);

            return ret;
        }

        pLogDebug("fstatat called for %s.", path);
        return -1;
    }

    int pipe(int pipefd[2])
    {
        return 0;
    }

    long sysconf(int name)
    {
        return 0;
    }

    char *realpath(const char* input, char* resolved_path)
    {
        resolved_path = input;
        return input;
    }
}
#endif