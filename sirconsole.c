/*!
 * \file sirconsole.h
 * \brief Internal implementation of console color management in the SIR library.
 * \author Ryan Matthew Lederman <lederman@gmail.com>
 */
#include "sirconsole.h"
#include "sirmacros.h"
#include "sirtextstyle.h"

/* \cond PRIVATE */

#ifndef _WIN32

static bool _sir_write_std(const sirchar_t* message, FILE* stream);

bool _sir_stderr_write(const sirchar_t* message) {
    return _sir_write_std(message, stderr);
}

bool _sir_stdout_write(const sirchar_t* message) {
    return _sir_write_std(message, stdout);
}

static bool _sir_write_std(const sirchar_t* message, FILE* stream) {

    assert(validstr(message));
    assert(stream);

    int write = validstr(message) ? fputs(message, stream) : -1;
    assert(write >= 0);

    return write >= 0;
}

#else

static bool _sir_write_stdwin32(uint16_t style, const sirchar_t* message, HANDLE console);

bool _sir_stderr_write(uint16_t style, const sirchar_t* message) {
    return _sir_write_stdwin32(style, message, GetStdHandle(STD_ERROR_HANDLE));
}

bool _sir_stdout_write(uint16_t style, const sirchar_t* message) {   
    return _sir_write_stdwin32(style, message, GetStdHandle(STD_OUTPUT_HANDLE));
}

static bool _sir_write_stdwin32(uint16_t style, const sirchar_t* message, HANDLE console) {

    assert(validstr(message));
    assert(INVALID_HANDLE_VALUE != console);

    if (INVALID_HANDLE_VALUE == console) {
        _sir_handleerr(GetLastError());
        return false;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbfi = {0};

    if (!GetConsoleScreenBufferInfo(console, &csbfi)) {
        _sir_handleerr(GetLastError());
        return false;
    }

    if (!SetConsoleTextAttribute(console, style)) {
        _sir_handleerr(GetLastError());
        return false;
    }        

    size_t chars = strnlen(message, SIR_MAXOUTPUT);
    DWORD written = 0;

    do {
        DWORD pass = 0;

        if (!WriteConsole(console, message, chars, &pass, NULL)) {
            _sir_handleerr(GetLastError());
            break;
        }

        written += pass;
    } while (written < chars);

    SetConsoleTextAttribute(console, csbfi.wAttributes);

    return written == chars;    
}

#endif  /* !_WIN32 */

/* \endcond PRIVATE */
