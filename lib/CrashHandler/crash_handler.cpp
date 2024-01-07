/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

#ifndef SDL_HEAD_HHHHHH
#   include "sdl_proxy/sdl_head.h"
#endif
#ifndef SDL_SDL_ASSERT_H
#   include "sdl_proxy/sdl_assert.h"
#endif

#ifndef THEXTECH_NO_SDL_BUILD
#include <SDL2/SDL_version.h>

#   if !defined(THEXTECH_CLI_BUILD) && !defined(CUSTOM_AUDIO)
#   include <SDL2/SDL_mixer_ext.h>
#   endif
#endif

#include <exception>
#include <cstdlib>
#include <signal.h>

#include <lib/CrashHandler/backtrace.h>

#if defined(DEBUG_BUILD) && (Backtrace_FOUND || defined(_WIN32))
#define PGE_ENGINE_DEBUG
#endif

#ifdef PGE_ENGINE_DEBUG

#ifdef __gnu_linux__
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#endif //PGE_ENGINE_DEBUG

// Exclude platforms that don't have SIG_INFO support
#if    !defined(_WIN32) \
    && !defined(__3DS__) \
    && !defined(__WII__) \
    && !defined(__WIIU__) \
    && !defined(__SWITCH__) \
    && !defined(VITA)
#   define HAS_SIG_INFO
#endif

// Exclude personal data removal from platforms where API doesn't allows to recognise the user and/or home directory
#if    !defined(VITA) \
    && !defined(__3DS__) \
    && !defined(__WII__) \
    && !defined(__WIIU__)  \
    && !defined(__SWITCH__) \
    && !defined(__EMSCRIPTEN__) \
    && !defined(__ANDROID__) \
    && !defined(__HAIKU__)
#   define DO_REMOVE_PERSONAL_DATA
#endif


#ifdef PGE_ENGINE_DEBUG
static int isDebuggerPresent()
{
#if defined(__gnu_linux__)
    const size_t buf_size = 2048;
    char buf[buf_size];
    int debugger_present = 0;

    int status_fd = open("/proc/self/status", O_RDONLY);
    if(status_fd == -1)
        return 0;

    ssize_t num_read = read(status_fd, buf, buf_size);

    if(num_read > 0)
    {
        static const char TracerPid[] = "TracerPid:";
        char *tracer_pid;

        if(num_read < (ssize_t)buf_size)
            buf[num_read] = 0;
        else
            buf[buf_size - 1] = 0;
        tracer_pid    = strstr(buf, TracerPid);
        if(tracer_pid)
            debugger_present = static_cast<bool>(SDL_atoi(tracer_pid + sizeof(TracerPid) - 1));
    }
    return debugger_present;

#elif defined(__APPLE__)
    // https://stackoverflow.com/a/2200786/5618998
    int                 junk;
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.

    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.

    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    SDL_assert(junk == 0);

    // We're being debugged if the P_TRACED flag is set.

    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );

#elif defined(_WIN32)
    return IsDebuggerPresent();

#else
    return 0; // Unknown platform
#endif
}
#endif

#ifdef _WIN32
#   include <windows.h>
#   include <dbghelp.h>
#   include <shlobj.h>
#elif (defined(__linux__) && !defined(__ANDROID__) || defined(__APPLE__))
#   include <pwd.h>
#   include <unistd.h>
#elif defined(__ANDROID__)
#   include <unwind.h>
#   include <dlfcn.h>
#   include <sstream>
#   include <iomanip>
#endif

#include "crash_handler.h"
#include "../Logger/logger.h"

#include "../../version.h"
#include "core/msgbox.h"

#define STACK_FORMAT    \
    "====Stack trace====\n" \
    "%s\n" \
    "===================\n" \
    "%s"

#define STR_EXPAND(tok) #tok
#define STRR(tok) STR_EXPAND(tok)

static const char *g_messageToUser =
    "================================================\n"
    "            Additional information:\n"
    "================================================\n"
    V_FILE_DESC "\n"
    "- Version:           " V_FILE_VERSION V_FILE_RELEASE "\n"
    "- Architecture:      " FILE_CPU "\n"
    "- Operating system:  " OPERATION_SYSTEM "\n"
    "- GIT Revision code: #" V_BUILD_VER "\n"
    "- GIT branch:        " V_BUILD_BRANCH "\n"
    "- Build date:        " V_DATE_OF_BUILD "\n"
    "================================================\n"
#ifndef THEXTECH_NO_SDL_BUILD
    "SDL2 version:        " STRR(SDL_MAJOR_VERSION) "." STRR(SDL_MINOR_VERSION) "." STRR(SDL_PATCHLEVEL) "\n"
#endif
#if !defined(THEXTECH_NO_SDL_BUILD) && !defined(THEXTECH_CLI_BUILD) && !defined(CUSTOM_AUDIO)
    "SDL Mixer X version: " STRR(SDL_MIXER_MAJOR_VERSION) "." STRR(SDL_MIXER_MINOR_VERSION) "." STRR(SDL_MIXER_PATCHLEVEL) "\n"
#endif
    "================================================\n"
    " Please send this log file to the developers by one of ways:\n"
    " - Via contact form:          https://wohlsoft.ru/contacts/\n"
    " - Official forums:           https://wohlsoft.ru/forum/\n"
    " - Official Discord server:   https://wohlsoft.ru/chat/\n"
    " - Make issue at GitHub repo: https://github.com/Wohlstand/TheXTech\n\n"
    "================================================\n";

#ifdef _WIN32
//
// http://blog.aaronballman.com/2011/04/generating-a-stack-crawl/
//
static bool GetStackWalk(std::string &outWalk)
{
    // Set up the symbol options so that we can gather information from the current
    // executable's PDB files, as well as the Microsoft symbol servers.  We also want
    // to undecorate the symbol names we're returned.  If you want, you can add other
    // symbol servers or paths via a semi-colon separated list in SymInitialized.
    ::SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME);

    if(!::SymInitialize(::GetCurrentProcess(), "http://msdl.microsoft.com/download/symbols", TRUE)) return false;

    // Capture up to 25 stack frames from the current call stack.  We're going to
    // skip the first stack frame returned because that's the GetStackWalk function
    // itself, which we don't care about.
    PVOID addrs[ 400 ] = { 0 };
    USHORT frames = CaptureStackBackTrace(1, 400, addrs, nullptr);

    for(USHORT i = 0; i < frames; i++)
    {
        // Allocate a buffer large enough to hold the symbol information on the stack and get
        // a pointer to the buffer.  We also have to set the size of the symbol structure itself
        // and the number of bytes reserved for the name.
        ULONG64 buffer[(sizeof(SYMBOL_INFO) + 1024 + sizeof(ULONG64) - 1) / sizeof(ULONG64) ] = { 0 };
        SYMBOL_INFO *info = (SYMBOL_INFO *)buffer;
        info->SizeOfStruct = sizeof(SYMBOL_INFO);
        info->MaxNameLen = 1024;
        // Attempt to get information about the symbol and add it to our output parameter.
        DWORD64 displacement = 0;

        if(::SymFromAddr(::GetCurrentProcess(), (DWORD64)addrs[ i ], &displacement, info))
        {
            outWalk.append(info->Name, info->NameLen);
            outWalk.append("\n");
        }
    }

    ::SymCleanup(::GetCurrentProcess());
    return true;
}
#endif

#ifdef __ANDROID__
namespace AndroidStackTrace
{
struct BacktraceState
{
    void **current;
    void **end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context, void *arg)
{
    BacktraceState *state = static_cast<BacktraceState *>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if(pc)
    {
        if(state->current == state->end)
            return _URC_END_OF_STACK;
        else
            *state->current++ = reinterpret_cast<void *>(pc);
    }
    return _URC_NO_REASON;
}

}

static size_t captureBacktrace(void **buffer, size_t max)
{
    AndroidStackTrace::BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(AndroidStackTrace::unwindCallback, &state);

    return state.current - buffer;
}

static void androidDumpBacktrace(std::ostringstream &os, void **buffer, size_t count)
{
    for(size_t idx = 0; idx < count; ++idx)
    {
        const void *addr = buffer[idx];
        const char *symbol = "";

        Dl_info info;
        if(dladdr(addr, &info) && info.dli_sname)
            symbol = info.dli_sname;

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
    }
}
#endif

#ifdef DO_REMOVE_PERSONAL_DATA
static std::string getCurrentUserName()
{
    std::string user;

#if defined(_WIN32)
#   if defined(THEXTECH_WINRT)
    user = "UnknownUser";  // FIXME: Implement for WinRT
#   else
    char    userName[256];
    wchar_t userNameW[256];
    DWORD usernameLen = 256;
    GetUserNameW(userNameW, &usernameLen);
    userNameW[usernameLen--] = L'\0';
    size_t nCnt = WideCharToMultiByte(CP_UTF8, 0, userNameW, usernameLen, userName, 256, 0, 0);
    userName[nCnt] = '\0';
    user = std::string(userName);
#   endif
#else
    struct passwd *pwd = getpwuid(getuid());
    if(pwd == nullptr)
        return "UnknownUser"; // Failed to get a user name!
    user = std::string(pwd->pw_name);
#endif

    return user;
}

static std::string getCurrentHomePath()
{
    std::string homedir;

#ifdef _WIN32
#   if defined(THEXTECH_WINRT)
    homedir = "C:\\Users\\<unknown>"; // FIXME: Implement for WinRT
#   else
    char    homeDir[MAX_PATH * 4];
    wchar_t homeDirW[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, homeDirW);
    size_t nCnt = WideCharToMultiByte(CP_UTF8, 0, homeDirW, -1, homeDir, MAX_PATH * 4, 0, 0);
    homeDir[nCnt] = '\0';
    homedir = std::string(homeDir);
#   endif
#elif defined(__HAIKU__)
    {
        const char *home = SDL_getenv("HOME");
        if(home == nullptr)
            return "/home/<unknown>"; // Failed to get a user name!
        homedir = std::string(home);
    }
#else
    struct passwd *pwd = getpwuid(getuid());
    if(pwd == nullptr)
        return "/home/<unknown>"; // Failed to get a user name!
    homedir = std::string(pwd->pw_dir);

#endif

    return homedir;
}

static void replaceStr(std::string &data, const std::string &toSearch, const std::string &replaceStr)
{
    size_t pos = data.find(toSearch);

    while(pos != std::string::npos)
    {
        data.replace(pos, toSearch.size(), replaceStr);
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}

static void removePersonalData(std::string &log)
{
    std::string user = getCurrentUserName();
    std::string homePath = getCurrentHomePath();

    // Replace username
    if(!homePath.empty())
    {
        replaceStr(log, homePath, "{...}");
#ifdef _WIN32
        replaceStr(homePath, "\\", "/");
        replaceStr(log, homePath, "{...}");
#endif
    }

    replaceStr(log, user, "anonymouse");
}

#endif // DO_REMOVE_PERSONAL_DATA

#if defined(__WII__) || defined(__WIIU__)
#   define USE_PPC_BACKTRACE
int ppc_backtrace(void **buffer, int size)
{
    int depth;
    uint32_t stackptr = 0, lr, *addr;

    // get link register
    asm volatile ("mflr %0" : "=r"(lr));

    // link register is assigned to depth[0]
    buffer[0] = (void *) (lr - 4);

    // get stackpointer
    asm volatile("stw %%sp, 0(%0)" : : "b" ((uint32_t)&stackptr));

    // assign stack ptr to address
    addr = reinterpret_cast<uint32_t *>(stackptr);

    // get the frames
    if (*addr)
    {
        // skip first two frames because this function
        // does create a stackframe but doesn't set lr on
        // the previous one.
        addr = (uint32_t *)*addr;
        if (*addr)
            addr = (uint32_t *)*addr;

    }

    for(depth = 1; (depth < size && *addr); ++depth)
    {
        uint32_t * next = (uint32_t *) *addr;
        buffer[depth] = (void *) (*(addr + 1) - 4);
        addr = next;
    }

    return depth;
}
#endif

static std::string getStacktrace()
{
    D_pLogDebugNA("Initializing std::string...");
    std::string bkTrace;

#if defined(_WIN32)
    GetStackWalk(bkTrace);

#elif defined(USE_PPC_BACKTRACE)
    void  *array[400];
    char stack_entry[25];
    int size;

    D_pLogDebugNA("Requesting backtrace...");
    size = ppc_backtrace(array, 400);

    D_pLogDebugNA("Filling std::string...");
    for(int j = 0; j < size; j++)
    {
        SDL_snprintf(stack_entry, 25, "- 0x%08x\n", (uint32_t)array[j]);
        bkTrace.append(stack_entry);
    }

    D_pLogDebugNA("DONE!");

#elif Backtrace_FOUND
    void  *array[400];
    int size;
    char **strings;

    D_pLogDebugNA("Requesting backtrace...");
    size = backtrace(array, 400);
    D_pLogDebugNA("Converting...");
    strings = backtrace_symbols(array, size);
    D_pLogDebugNA("Filling std::string...");

    for(int j = 0; j < size; j++)
    {
        bkTrace.append(strings[j]);
        bkTrace.push_back('\n');
    }

    D_pLogDebugNA("DONE!");

#elif defined(__ANDROID__)
    const size_t max = 400;
    void *buffer[max];
    std::ostringstream oss;
    androidDumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
    D_pLogDebugNA("DONE!");
    bkTrace = oss.str();

#else
    bkTrace = "<Stack trace not supported for this platform!>";
#endif

#ifdef DO_REMOVE_PERSONAL_DATA
    removePersonalData(bkTrace);
#endif // DO_REMOVE_PERSONAL_DATA

    return bkTrace;
}


#ifdef __GNUC__
#   define LLVM_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#   define LLVM_ATTRIBUTE_NORETURN //__declspec(noreturn)
#else
#   define LLVM_ATTRIBUTE_NORETURN
#endif

static LLVM_ATTRIBUTE_NORETURN void abortEngine(int signal)
{
    CloseLog();
#ifndef THEXTECH_NO_SDL_BUILD
    SDL_Quit();
#endif
    exit(signal);
}

void LLVM_ATTRIBUTE_NORETURN CrashHandler::crashByUnhandledException()
{
    std::string stack = getStacktrace();
    std::string exc;

    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch(const std::exception &e)
    {
        exc.append(" caught an unhandled exception. [");
        exc.append(e.what());
        exc.append("]");
    }
    catch(...)
    {
        exc.append(" caught unhandled exception. (unknown) ");
    }

    pLogFatal("<Unhandled exception! %s>\n"
              STACK_FORMAT, exc.c_str(),
              stack.c_str(), g_messageToUser);
    XMsgBox::errorMsgBox(
        //% "Unhandled exception!"
        "Unhandled exception!",
        //% "Engine has crashed because accepted unhandled exception!"
        "Engine has crashed because accepted unhandled exception!");
    abortEngine(-1);
}

void LLVM_ATTRIBUTE_NORETURN CrashHandler::crashByFlood()
{
    std::string stack = getStacktrace();
    pLogFatal("<Out of memory!>\n"
              STACK_FORMAT,
              stack.c_str(), g_messageToUser);
    XMsgBox::errorMsgBox(
        //% "Out of memory!"
        "Out of memory!",
        //% "Engine has crashed because out of memory! Try to close other applications and restart game."
        "Engine has crashed because out of memory! Try to close other applications and restart game.");
    abortEngine(-2);
}

#if defined(_WIN32) // Unsupported signals by Windows
struct siginfo_t;
#endif

static void handle_signal(int signal, siginfo_t *siginfo, void * /*context*/)
{
#if !defined(_WIN32) || !defined(__SWITCH__)  // Unsupported signals by Windows and Switch
    (void)siginfo;
#endif

    // Find out which signal we're handling
    switch(signal)
    {
#ifndef _WIN32  //Unsupported signals by Windows

    case SIGHUP:
        pLogWarning("Terminal was closed");
        abortEngine(signal);

    case SIGQUIT:
        pLogWarning("<Quit command>");
        abortEngine(signal);

    case SIGKILL:
        pLogFatal("<killed>");
        abortEngine(signal);

    case SIGALRM:
    {
        pLogFatal("<alarm() time out!>");
        XMsgBox::errorMsgBox(
            //% "Time out!"
            "Time out!",
            //% "Engine has abourted because alarm() time out!"
            "Engine has abourted because alarm() time out!");
        abortEngine(signal);
    }

    case SIGBUS:
    {
        std::string stack = getStacktrace();

#   if defined(HAS_SIG_INFO)
        if(siginfo)
        {
            switch(siginfo->si_code)
            {
            case BUS_ADRALN:
                pLogFatal("<Physical memory address error: wrong address alignment>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case BUS_ADRERR:
                pLogFatal("<Physical memory address error: physical address is not exists>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case BUS_OBJERR:
                pLogFatal("<Physical memory address error: object specific hardware error>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            default:
                pLogFatal("<Physical memory address error>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
            }
        }
        else
#   endif // HAS_SIG_INFO
        {
            pLogFatal("<Physical memory address error>\n"
                      STACK_FORMAT,
                      stack.c_str(), g_messageToUser);
        }

        XMsgBox::errorMsgBox(
            //% "Physical memory address error!"
            "Physical memory address error!",
            //% "Engine has crashed because a physical memory address error"
            "Engine has crashed because a physical memory address error");
        abortEngine(signal);
    }

    case SIGURG:
    case SIGUSR1:
    case SIGUSR2:
        break;

    case SIGILL:
    {
        std::string stack = getStacktrace();
        pLogFatal("<Wrong CPU Instruction>\n"
                  STACK_FORMAT,
                  stack.c_str(), g_messageToUser);
        XMsgBox::errorMsgBox(
            //% "Wrong CPU Instruction!"
            "Wrong CPU Instruction!",
            //% "Engine has crashed because a wrong CPU instruction"
            "Engine has crashed because a wrong CPU instruction");
        abortEngine(signal);
    }

#endif // NOT _WIN32

    case SIGFPE:
    {
        std::string stack = getStacktrace();

#if defined(HAS_SIG_INFO)  //Unsupported signals by Windows and Switch
        if(siginfo)
        {
            switch(siginfo->si_code)
            {
            case FPE_INTDIV:
                pLogFatal("<wrong arithmetical operation: division of integer number by zero>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case FPE_FLTDIV:
                pLogFatal("<wrong arithmetical operation: division of floating point number by zero>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case FPE_INTOVF:
                pLogFatal("<wrong arithmetical operation: integer number max bits size overflot>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case FPE_FLTOVF:
                pLogFatal("<wrong arithmetical operation: floating point number max bits size overflot>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            default:
                pLogFatal("<wrong arithmetical operation>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
            }
        }
        else
#endif // HAS_SIG_INFO
        {
            pLogFatal("<wrong arithmetical operation>\n"
                      STACK_FORMAT,
                      stack.c_str(), g_messageToUser);
        }

        XMsgBox::errorMsgBox(
            //% "Wrong arithmetical operation"
            "Wrong arithmetical operation",
            //% "Engine has crashed because of a wrong arithmetical operation!"
            "Engine has crashed because of a wrong arithmetical operation!");
        abortEngine(signal);
    }

    case SIGABRT:
    {
        std::string stack = getStacktrace();
        pLogFatal("<Aborted!>\n"
                  STACK_FORMAT,
                  stack.c_str(), g_messageToUser);
        XMsgBox::errorMsgBox(
            //% "Aborted"
            "Aborted",
            //% "Engine has been aborted because critical error was occouped."
            "Engine has been aborted because critical error was occouped.");
        abortEngine(signal);
    }

    case SIGSEGV:
    {
        D_pLogDebugNA("\n===========================================================\n"
                      "Attempt to take a backtrace..."
                      "(if log ends before \"DONE\" will be shown, seems also trouble in the backtracing function too...)");
        std::string stack = getStacktrace();

#if defined(HAS_SIG_INFO)  //Unsupported signals by Windows
        if(siginfo)
        {
            switch(siginfo->si_code)
            {
            case SEGV_MAPERR:
                pLogFatal("<Segmentation fault crash!: Address is not pointing to object!!!>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            case SEGV_ACCERR:
                pLogFatal("<Segmentation fault crash!: Wrong access rights for address!!!>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;

            default:
                pLogFatal("<Segmentation fault crash!>\n"
                          STACK_FORMAT,
                          stack.c_str(), g_messageToUser);
                break;
            }
        }
        else
#endif // HAS_SIG_INFO
        {
            pLogFatal("<Segmentation fault crash!>\n"
                      STACK_FORMAT,
                      stack.c_str(), g_messageToUser);
        }

        XMsgBox::errorMsgBox(
            //% "Segmentation fault"
            "Segmentation fault",
            /*% "Engine has crashed because of a Segmentation fault.\n"
                "Run debugging with a built in debug mode application\n"
                "and retry your recent actions to get more detailed information." */
            "Engine has crashed because of a Segmentation fault.\n"
            "Run debugging with a built in debug mode application\n"
            "and retry your recent actions to get more detailed information.");
        abortEngine(signal);
    }

    case SIGINT:
    {
        pLogFatal("<Interrupted!>");
        XMsgBox::errorMsgBox(
            //% "Interrupt"
            "Interrupt",
            //% "Engine has been interrupted"
            "Engine has been interrupted");
        abortEngine(signal);
    }

    default:
        return;
    }
}

#ifndef THEXTECH_NO_SDL_BUILD
static SDL_AssertState custom_sdl_handler(const SDL_AssertData *data, void *userdata)
{
    std::string stack = getStacktrace();
    pLogFatal("<Assertion condition has failed>:\n"
              "---------------------------------------------------------\n"
              "File: %s(%d)\n"
              "Function: %s\n"
              "Condition: %s\n"
              "---------------------------------------------------------\n"
              STACK_FORMAT,
              data->filename, data->linenum,
              data->function,
              data->condition,
              stack.c_str(),
              g_messageToUser);

    return SDL_GetDefaultAssertionHandler()(data, userdata);
}
#endif

#if defined(HAS_SIG_INFO)
static struct sigaction act;
#else
#   if _WIN32
struct siginfo_t;
#   endif

static void handle_signalWIN32(int signal)
{
    handle_signal(signal, nullptr, nullptr);
}
#endif


void CrashHandler::initSigs()
{
#ifdef PGE_ENGINE_DEBUG
    if(isDebuggerPresent())
        return; // Don't initialize crash handlers on attached debugger
#endif

#ifndef THEXTECH_NO_SDL_BUILD
    SDL_SetAssertionHandler(&custom_sdl_handler, NULL);
#endif

    std::set_new_handler(&crashByFlood);
    std::set_terminate(&crashByUnhandledException);
#if defined(HAS_SIG_INFO) // Unsupported signals by Windows
    memset(&act, 0, sizeof(struct sigaction));
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handle_signal;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP,  &act, nullptr);
    sigaction(SIGQUIT, &act, nullptr);
    //sigaction(SIGKILL, &act, nullptr); This signal is unhandlable
    sigaction(SIGALRM, &act, nullptr);
    sigaction(SIGURG,  &act, nullptr);
    sigaction(SIGUSR1, &act, nullptr);
    sigaction(SIGUSR2, &act, nullptr);
    sigaction(SIGBUS,  &act, nullptr);
    sigaction(SIGILL,  &act, nullptr);
    sigaction(SIGFPE,  &act, nullptr);
    sigaction(SIGSEGV, &act, nullptr);
    sigaction(SIGINT,  &act, nullptr);
    sigaction(SIGABRT, &act, nullptr);
#else // HAS_SIG_INFO
    signal(SIGILL,  &handle_signalWIN32);
    signal(SIGFPE,  &handle_signalWIN32);
    signal(SIGSEGV, &handle_signalWIN32);
    signal(SIGINT,  &handle_signalWIN32);
    signal(SIGABRT, &handle_signalWIN32);
#   ifndef _WIN32
    signal(SIGHUP, &handle_signalWIN32);
    signal(SIGQUIT, &handle_signalWIN32);
    signal(SIGALRM, &handle_signalWIN32);
    signal(SIGBUS, &handle_signalWIN32);
    signal(SIGURG, &handle_signalWIN32);
#   endif
#endif // HAS_SIG_INFO
}
/* Signals End */
