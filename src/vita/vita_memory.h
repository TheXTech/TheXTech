#ifndef __VITA_MEMORY_H__
#define __VITA_MEMORY_H__

#define MEMORY_SCELIBC_MB 120
#define MEMORY_NEWLIB_MB 120

int sceLibcHeapSize = MEMORY_SCELIBC_MB * 1024 * 1024;
int _newlib_heap_size_user = MEMORY_NEWLIB_MB * 1024 * 1024;

#endif