/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2025 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef SDL_PROXY_RWOPS_H
#define SDL_PROXY_RWOPS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* RWops Types */
#define SDL_RWOPS_UNKNOWN   0U  /**< Unknown stream type */
#define SDL_RWOPS_STDFILE   2U  /**< Stdio file */

#define RW_SEEK_CUR SEEK_CUR
#define RW_SEEK_SET SEEK_SET
#define RW_SEEK_END SEEK_END

typedef struct SDL_RWops
{
    /**
     *  Return the size of the file in this rwops, or -1 if unknown
     */
    int64_t (* size) (struct SDL_RWops * context);

    /**
     *  Seek to `offset` relative to `whence`, one of stdio's whence values:
     *  RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
     *
     *  \return the final offset in the data stream, or -1 on error.
     */
    int64_t (* seek) (struct SDL_RWops * context, int64_t offset,
                             int whence);

    /**
     *  Read up to `maxnum` objects each of size `size` from the data
     *  stream to the area pointed at by `ptr`.
     *
     *  \return the number of objects read, or 0 at error or end of file.
     */
    size_t (* read) (struct SDL_RWops * context, void *ptr,
                             size_t size, size_t maxnum);

    /**
     *  Write exactly `num` objects each of size `size` from the area
     *  pointed at by `ptr` to data stream.
     *
     *  \return the number of objects written, or 0 at error or end of file.
     */
    size_t (* write) (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);

    /**
     *  Close and free an allocated SDL_RWops structure.
     *
     *  \return 0 if successful or -1 on write error when flushing data.
     */
    int (* close) (struct SDL_RWops * context);

    int type;

    union
    {
      struct
      {
          void *data1;
      } unknown;
    } hidden;
} SDL_RWops;

SDL_RWops* SDL_AllocRW();
void SDL_FreeRW(SDL_RWops * area);
SDL_RWops* SDL_RWFromFile(const char* pathname, const char* mode);

#ifdef __cplusplus
} // extern "C"
#endif

/* C declarations or C++ inline definitions of standard RWops functions */
#include "_priv_rwops_funcs.h"

#endif /* #ifndef SDL_PROXY_RWOPS_H */
