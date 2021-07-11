#if VITA
#include "dirman_vita.h"
int dirfd(DIR *dirp)
    {
        return 0;
    }
extern "C" {
    int mkdir(const char *path, int mode)
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

    int fstatat(int fd, const char* __restrict path,
       struct stat* __restrict buf, int flag)
    {

        return 0;
    }

    int pipe(int pipefd[2])
    {
        return 0;
    }

    long sysconf(int name)
    {
        return 0;
    }

    char *realpath(const char *__restrict input, char *__restrict resolved_path)
    {
        return (char *)input;
    }
}
#endif