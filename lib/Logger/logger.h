/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2020 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
#include <string>

enum class PGE_LogLevel
{
    Debug    = 5,
    Warning  = 4,
    Critical = 3,
    Info     = 2,
    Fatal    = 1,
    NoLog    = 0,
};

extern void LoadLogSettings(bool disableStdOut = false, bool verboseLogs = false);
extern void CloseLog();
#endif//__cplusplus

#ifdef __cplusplus
extern "C"
{
#endif
extern void pLogDebug(const char *format, ...);
extern void pLogWarning(const char *format, ...);
extern void pLogCritical(const char *format, ...);
extern void pLogInfo(const char *format, ...);
extern void pLogFatal(const char *format, ...);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern void WriteToLog(PGE_LogLevel type, const std::string &msg);
extern std::string getLogFilePath();
#endif

#ifdef DEBUG_BUILD
// Variadic with arguments
#define D_pLogDebug(fmt, ...) pLogDebug(fmt, ##__VA_ARGS__)
#define D_pLogWarning(fmt, ...) pLogWarning(fmt, ##__VA_ARGS__)
#define D_pLogCritical(fmt, ...) pLogCritical(fmt, ##__VA_ARGS__)
#define D_pLogInfo(fmt, ...) pLogInfo(fmt, ##__VA_ARGS__)
#define D_pLogFatal(fmt, ...) pLogFatal(fmt, ##__VA_ARGS__)
// Standard without arguments to avoid "ISO C++11 requires at least one argument of the '...' in a variadic macro"
#define D_pLogDebugNA(fmt) pLogDebug(fmt)
#define D_pLogWarningNA(fmt) pLogWarning(fmt)
#define D_pLogCriticalNA(fmt) pLogCritical(fmt)
#define D_pLogInfoNA(fmt) pLogInfo(fmt)
#define D_pLogFatalNA(fmt) pLogFatal(fmt)
#else
#define D_pLogDebug(fmt, ...)
#define D_pLogWarning(fmt, ...)
#define D_pLogCritical(fmt, ...)
#define D_pLogFatal(fmt, ...)
#define D_pLogInfo(fmt, ...)
#define D_pLogDebugNA(fmt)
#define D_pLogWarningNA(fmt)
#define D_pLogCriticalNA(fmt)
#define D_pLogFatalNA(fmt)
#define D_pLogInfoNA(fmt)
#endif

#ifdef __cplusplus
#define LogDebug(msg) WriteToLog(PGE_LogLevel::Debug, msg)
#define LogWarning(msg) WriteToLog(PGE_LogLevel::Warning, msg)
#define LogCritical(msg) WriteToLog(PGE_LogLevel::Critical, msg)
#define LogFatal(msg) WriteToLog(PGE_LogLevel::Fatal, msg)
#endif//__cplusplus

#endif // LOGGER_H
