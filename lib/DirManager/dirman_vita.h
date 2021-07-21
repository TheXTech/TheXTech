#ifndef __DIRMAN_VITA_H__
#define __DIRMAN_VITA_H__

#ifdef VITA
#include <sys/dirent.h>

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>


int dirfd(DIR_ *dirp);
#endif

#endif //__DIRMAN_VITA_H__