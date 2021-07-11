#ifndef __DIRMAN_VITA_H__
#define __DIRMAN_VITA_H__

#if VITA
#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <dirent.h>

int dirfd(DIR *dirp);
#endif

#endif //__DIRMAN_VITA_H__