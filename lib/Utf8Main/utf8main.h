/*
 * utf8main - a small wrapper over entry points
 * to provide the UTF8 encoded main function
 *
 * Copyright (c) 2017-2024 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef UTF8MAIN_H
#define UTF8MAIN_H

#ifndef PGE_ENGINE//Don't use in PGE Engine where SDL Main is already in use
//Windows platform
#ifdef _WIN32
//Avoid Qt to steal Main function
#if defined(QT_NEEDS_QMAIN)
#undef main
#endif
//Define custom UTF8 main function which will convert command line arguments into UTF8 and will pass them
#define main UTF8_Main
#endif

#endif

#endif // UTF8MAIN_H
