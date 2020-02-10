/*
 * A small set of additional math functions and templates
 *
 * Copyright (c) 2017-2020 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "open_url.h"

/*
    WinAPI implementation
*/
#ifdef _WIN32
#define OPENURL_SUPPORTED
#include <windows.h>
#include <shellapi.h>

void Utils::openUrl(const std::string &url)
{
    std::wstring urlW;
    urlW.resize(url.size());
    int newlen = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), url.length(), &urlW[0], urlW.length());
    urlW.resize(newlen);
    ShellExecuteW(NULL, L"open", urlW.c_str(), NULL, NULL, SW_SHOW);
}
#endif

/*
    Cocoa implementation
*/
#ifdef __APPLE__
#define OPENURL_SUPPORTED
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>

void Utils::openUrl(const std::string &url)
{
    CFURLRef cfurl = CFURLCreateWithBytes(
                         NULL,                        // allocator
                         (UInt8 *)url.c_str(),    // URLBytes
                         url.length(),            // length
                         kCFStringEncodingASCII,      // encoding
                         NULL                         // baseURL
                     );
    LSOpenCFURLRef(cfurl, 0);
    CFRelease(cfurl);
}
#endif

/*
    Linux/FreeBSD implementation
*/
#if defined(__unix__) && (defined(__gnu_linux__) || defined(__FreeBSD__))
#define OPENURL_SUPPORTED
#include <stdlib.h>
#include <string.h>
#include "files.h"

static const char *defaultPaths[] =
{
    "/usr/local/bin/",
    "/usr/bin/",
    "/bin/"
};

static const char *browsers[] =
{
    "firefox",
    "chromium-browser",
    "google-chrome",
    "mozilla",
    "opera"
};

static bool findExec(std::string &exec, const std::string &target)
{
    size_t len = sizeof(defaultPaths)/sizeof(const char*);
    //Is absolute path
    if((target.compare(0, 1, "/") == 0) && (Files::fileExists(target)))
    {
        exec = target;
        return true;
    }

    for(size_t i = 0; i < len; i++)
    {
        if(Files::fileExists(defaultPaths[i] + target))
        {
            exec = defaultPaths[i] + target;
            return true;
        }
    }
    return false;
}

static void execApp(const std::string &prog, const std::string &args)
{
    std::string ex = prog + " " + args + " &";
    if(system(ex.c_str()) != 0)
    {
        fprintf(stderr, "Warning: Opening of URL %s finished with errors\n", args.c_str());
        fflush(stderr);
    }
}

void Utils::openUrl(const std::string &url)
{
    std::string browser;

    if(findExec(browser, "xdg-open"))
    {
        execApp(browser, url);
        return;
    }
    char* envBrowser = nullptr;
    char* envDesktop = nullptr;

    envBrowser = getenv("DEFAULT_BROWSER");
    if(envBrowser)
        envBrowser = getenv("BROWSER");
    if(envBrowser && findExec(browser, envBrowser))
    {
        execApp(browser, url);
        return;
    }

    envDesktop = getenv("XDG_CURRENT_DESKTOP");
    if(envDesktop)
    {
        if(strcmp(envDesktop, "KDE") == 0)
        {
            if(findExec(browser, "kfmclient"))
            {
                std::string args = " exec " + url;
                execApp(browser, args);
                return;
            }
        }

        if(strcmp(envDesktop, "GNOME") == 0)
        {
            if(findExec(browser, "gnome-open"))
            {
                execApp(browser, url);
                return;
            }
        }
    }

    {
        size_t len = sizeof(browsers)/sizeof(const char*);
        for(size_t i = 0; i < len; i++)
        {
            if(findExec(browser, browsers[i]))
            {
                execApp(browser, url);
                return;
            }
        }
    }
}
#endif

/*
    Haiku implementation
*/
#ifdef __HAIKU__
#define OPENURL_SUPPORTED
void Utils::openUrl(const std::string &url)
{
	(void)url;
    //FIXME: Implement this!
}
#endif

/*
    Dummy for unsupported operating systems
*/
#ifndef OPENURL_SUPPORTED
#include <stdio.h>

void Utils::openUrl(const std::string &url)
{
	(void)url;
    fprintf(stderr, "Warning: Opening of URLs by Utils::openUrl() is not supported on this operating system\n");
    fflush(stderr);
}
#endif

