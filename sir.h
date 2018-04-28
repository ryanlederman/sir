/*!
 * \file sir.h
 * \brief Definitions for the Standard Incident Reporter (SIR) library.
 *
 * \author Ryan Matthew Lederman <lederman@gmail.com>
 * \version 1.1.0
 * \date 2003-2018
 * \copyright MIT License
 */

#ifndef _SIR_H_INCLUDED
#define _SIR_H_INCLUDED

#include "sirplatform.h"
#include "sirmutex.h"

/*! The maximum number of log files that may be registered. */
#define SIR_MAXFILES 16

/*! The maximum number of characters that may be included in one message,
 * not including other parts of the output, like the timestamp and level. */
#define SIR_MAXMESSAGE 2048

/*! The size, in characters, of the buffer used to hold time format strings. */
#define SIR_MAXTIME 64

/*! The size, in characters, of the buffer used to hold level format strings. */
#define SIR_MAXLEVEL 16

/*! The size, in characters, of the buffer used to hold process/appname
 * format strings. */
#define SIR_MAXNAME 32

/*! The maximum size, in characters, of final formatted output. */
#define SIR_MAXOUTPUT SIR_MAXMESSAGE + SIR_MAXTIME + SIR_MAXLEVEL + SIR_MAXNAME

/*! A sensible (?) constraint for the limit of a file's path. Note that this value
 * is only used in the absence of PATH_MAX. */
#ifndef SIR_MAXPATH
#define SIR_MAXPATH 65535
#endif

/*! The value used to represent an invalid file identifier. */
#define SIR_INVALID (int)-1

/*! The default time stamp format string at the beginning of log messages;
 * generated by \a strftime. */
#define SIR_TIMEFORMAT "%H:%M:%S"

/*! The size, in characters, of the buffer used to hold current millisecond strings. */
#define SIR_MAXMSEC 16

/*! The default format for the current millisecond in time stamps;
 * Use ::SIRO_NOMSEC to disable. */
#define SIR_MSECFORMAT ".%03ld"

/*! The default clock used to obtain the current millisecond from
 * \a clock_gettime (if it's available). */
#define SIR_MSECCLOCK CLOCK_MONOTONIC

/*! The default time format string in file headers (see ::SIR_FHFORMAT);
 * generated by \a strftime.
 *
 * Example output from the default: `15:13:41 Sat 28 Apr 18 (-0600)`
 */
#define SIR_FHTIMEFORMAT "%H:%M:%S %a %d %b %y (%z)"

/*! The default format string written to a log file when logging begins
 * or the file is rolled. The \a %s format identifier is the current
 * date/time (see ::SIR_FHTIMEFORMAT) according to \a localtime.
 */
#define SIR_FHFORMAT "\n\n----- session begin (%s) -----\n\n"

/*! The string representation of the ::SIRL_EMERG level in output. */
#define SIRL_S_EMERG "EMER"

/*! The string representation of the ::SIRL_ALERT level in output. */
#define SIRL_S_ALERT "ALRT"

/*! The string representation of the ::SIRL_CRIT level in output. */
#define SIRL_S_CRIT "CRIT"

/*! The string representation of the ::SIRL_ERROR level in output. */
#define SIRL_S_ERROR "ERR"

/*! The string representation of the ::SIRL_WARN level in output. */
#define SIRL_S_WARN "WARN"

/*! The string representation of the ::SIRL_NOTICE level in output. */
#define SIRL_S_NOTICE "NOTF"

/*! The string representation of the ::SIRL_INFO level in output. */
#define SIRL_S_INFO "INFO"

/*! The string representation of the ::SIRL_DEBUG level in output. */
#define SIRL_S_DEBUG "DBG"

/*! Logging levels
 *
 * Each enum value corresponds to a function that sends output
 * to all destinations for that level (::sirdebug and pals).
 *
 * Since the values are also flags (i.e., powers of two), they can be combined
 * to filter different levels of importance but not strictly linearly.
 */
typedef enum {
    SIRL_EMERG  = 0x1,  /*!< Nuclear war, Armageddon, etc. */
    SIRL_ALERT  = 0x2,  /*!< Action required ASAP. */
    SIRL_CRIT   = 0x4,  /*!< Critical errors. */
    SIRL_ERROR  = 0x8,  /*!< Errors. */
    SIRL_WARN   = 0x10, /*!< Warnings that could likely be ignored. */
    SIRL_NOTICE = 0x20, /*!< Normal but significant. */
    SIRL_INFO   = 0x40, /*!< Informational messages. */
    SIRL_DEBUG  = 0x80, /*!< Verbose debugging output. */
    SIRL_ALL    = 0xff  /*!< Includes all logging levels. */
} sir_level;

/*! One or more ::sir_level, bitwise OR'd. */
typedef uint16_t sir_levels;

/*! Output destination options
 *
 * Set any desired flags on the corresponding member in the ::sirinit structure
 * to fine-tune the output formatting for each destination.
 *
 * \attention For \a syslog (if enabled), the time stamp, name, and level are always
 * stripped from the output, since \a syslog already supplies this information. These
 * options only apply to \a stderr, \a stdout, and log files.
 */
typedef enum {
    SIRO_DEFAULT = 0,     /*!< Use the default for this type of destination. */
    SIRO_NOLEVEL = 0x100, /*!< Do not include the human-readable logging level in output. */
    SIRO_NONAME  = 0x200, /*!< Do not include the process/app name in output. */

    /*! Do not include time stamps in output. */
    SIRO_NOTIME = 0x400,

    /*! Do not include milliseconds in time stamps. If \a not set, time stamps
     * in output to this destination will be appended with the current millisecond
     * in addition to the hour, minute, and second. (see ::SIR_MSECFORMAT)
     * If ::SIRO_NOTIME is set, this has no effect.
     *
     * \attention Only available if \a SIR_MSEC_TIMER is defined, which varies by platform.
     * It is never defined if \a _WIN32 is defined.
     */
    SIRO_NOMSEC = 0x800,

    /*! Don't write header messages when logging begins, or the file is rolled.
     * Only applicable to log files. Setting on any other destination
     * will have no effect. */
    SIRO_NOHDR = 0x1000,

    /*! Includes all other options; effectively disables all output formatting except
     * the original formatted message (does not include ::SIRO_NOHDR; set that flag
     * in addition to remove header messages).
     */
    SIRO_MSGONLY = 0xef00
} sir_option;

/*! One or more ::sir_option, bitwise OR'd. */
typedef uint16_t sir_options;

/*! The underlying type to use for characters in output. */
typedef char sirchar_t;

/*! \struct sirinit
 *
 * \brief Initialization data for the library.
 *
 * Allocate an instance of this struct and pass it to ::sir_init
 * in order to begin using the library.
 *
 * Don't forget to call ::sir_cleanup when you're done.
 */
typedef struct {
    /*! Logging levels (::sir_level) to route to \a stdout. */
    sir_levels stdOutLevels;

    /*! Output options (::sir_option) for \a stdout. Set to ::SIRO_DEFAULT (0) to use
     * the default options for this type of destination (i.e., stdio, file, syslog).
     */
    sir_options stdOutOptions;

    /*! Logging levels (::sir_level) to route to \a stderr. */
    sir_levels stdErrLevels;

    /*! Output options (::sir_option) for \a stderr. Set to ::SIRO_DEFAULT (0) to use
     * the default options for this type of destination (i.e., stdio, file, syslog)
     */
    sir_options stdErrOptions;

    /*! Logging levels (::sir_level) to route to \a syslog.
     *
     * \attention Only available if \a SIR_NO_SYSLOG is \a not defined in the preprocessor.
     * If \a _WIN32 is defined, \a SIR_NO_SYSLOG is automatically defined.
     */
    sir_levels sysLogLevels;

    /*! Whether or not to include the current process identifier in messages sent
     * to \a syslog (default: false).
     *
     * \attention Only available if \a SIR_NO_SYSLOG is \a not defined in the preprocessor.
     *  If \a _WIN32 is defined, \a SIR_NO_SYSLOG is automatically defined.
     */
    bool sysLogIncludePID;

    /*! A custom name to include in output (default: \a none). If ::SIRO_NONAME
     * is set in the options for a destination, no name is included in output.
     * Set to \a NULL for the default.
     */
    const sirchar_t* processName;

    /*! A custom time format passed to \a strftime to use in output
     * (default: ::SIR_TIMEFORMAT). If ::SIRO_NOTIME is set in the options
     * for a destination, no time is included in output. Set to \a NULL for the default.
     */
    const sirchar_t* timeFmt;

    /*! Used internally by the library to determine whether or not it is in
     * valid operational state.
     * 
     * \attention For internal library use only. Do not modify.
     */
    uint32_t _sirmagic;
} sirinit;

#ifdef __cplusplus
extern "C" {
#endif /* !__cplusplus */

/*! \fn int sir_init(const sirinit *si)
 *
 * \brief Initializes the library.
 *
 * You must call this function before any others in the library
 * or you're gonna have a bad time.
 *
 * \param[in] si Pointer a to sirinit struct containing initialization options.
 *
 * \return boolean success
 */
bool sir_init(const sirinit* si);

/*! \fn int sirdebug(const sirchar_t *format, ...)
 *
 * \brief Log a formatted debug-level message.
 *
 * All output destinations whose flags have ::SIRL_DEBUG set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sirdebug(const sirchar_t* format, ...);

/*! \fn int sirinfo(const sirchar_t *format, ...)
 *
 * \brief Log a formatted informational message.
 *
 * All output destinations whose flags have ::SIRL_INFO set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sirinfo(const sirchar_t* format, ...);

/*! \fn int sirnotice(const sirchar_t *format, ...)
 *
 * \brief Log a formatted notice message.
 *
 * All output destinations whose flags have ::SIRL_NOTICE set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sirnotice(const sirchar_t* format, ...);

/*! \fn int sirwarn(const sirchar_t *format, ...)
 *
 * \brief Log a formatted warning message.
 *
 * All output destinations whose flags have ::SIRL_WARN set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sirwarn(const sirchar_t* format, ...);

/*! \fn int sirerror(const sirchar_t *format, ...)
 *
 * \brief Log a formatted error message.
 *
 * All output destinations whose flags have ::SIRL_ERROR set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sirerror(const sirchar_t* format, ...);

/*! \fn int sircrit(const sirchar_t *format, ...)
 *
 * \brief Log a formatted critical message.
 *
 * All output destinations whose flags have ::SIRL_CRIT set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool sircrit(const sirchar_t* format, ...);

/*! \fn int siralert(const sirchar_t *format, ...)
 *
 * \brief Log a formatted alert message.
 *
 * All output destinations whose flags have ::SIRL_ALERT set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool siralert(const sirchar_t* format, ...);

/*! \fn int siremerg(const sirchar_t *format, ...)
 *
 * \brief Log a formatted emergency message.
 *
 * All output destinations whose flags have ::SIRL_EMERG set
 * will receive the formatted message.
 *
 * \param[in] format A printf-style format string.
 * \param[in] ... A list of arguments whose type match the format
 *  specifier at the same index in the format string.
 *
 * \return boolean success indicating whether or not all destinations
 * meant to receive this level were successfully processed.
 */
bool siremerg(const sirchar_t* format, ...);

/*! \fn void sir_cleanup()
 *
 * \brief Shuts down the library.
 *
 * Call to free all allocated memory and reset internal data structures
 * to their default state.
 *
 * \return none
 */
void sir_cleanup();

/*! \fn int sir_addfile(const sirchar_t* path, sir_levels levels, sir_options opts)
 *
 * \brief Adds a log file output destination.
 *
 * Adds a log file to be appended to whenever a message
 * matching any of the ::sir_level flags set in \a levels is emitted.
 *
 * \param[in] path The path to the log file. It will be created if it doesn't
 * exist.
 *
 * \param[in] levels One or more ::sir_level logging levels for which
 * output should be append to the file.
 *
 * \param[in] opts Zero or more ::sir_option flags to control output formatting.
 *
 * \return A unique identifier for the file, or ::SIR_INVALID (-1) if an error occurs.
 */
int sir_addfile(const sirchar_t* path, sir_levels levels, sir_options opts);

/*! \fn bool sir_remfile(int id)
 *
 * \brief Removes a previously added log file.
 *
 * \param[in] id The identifier returned from ::sir_addfile.
 *
 * \return boolean success
 */
bool sir_remfile(int id);

#ifdef __cplusplus
}
#endif /* !__cplusplus */

/*! \cond PRIVATE */

typedef struct {
    sirchar_t*  path;
    sir_levels  levels;
    sir_options opts;
    FILE*       f;
    int         id;
} sirfile;

typedef struct {
    sirfile* files[SIR_MAXFILES];
    size_t   count;
} sirfiles;

typedef struct {
    sirchar_t* timestamp;
    sirchar_t* msec;
    sirchar_t* level;
    sirchar_t* name;
    sirchar_t* message;
    sirchar_t* output;
} siroutput;

typedef struct {
    sirchar_t timestamp[SIR_MAXTIME];
    sirchar_t msec[SIR_MAXMSEC];
    sirchar_t level[SIR_MAXLEVEL];
    sirchar_t name[SIR_MAXNAME];
    sirchar_t message[SIR_MAXMESSAGE];
    sirchar_t output[SIR_MAXOUTPUT];
} sirbuf;

#define _SIR_MAGIC 0x0

#define _SIRBUF_TIME 0
#define _SIRBUF_MSEC 1
#define _SIRBUF_LEVEL 2
#define _SIRBUF_NAME 3
#define _SIRBUF_MSG 4
#define _SIRBUF_OUTPUT 5
#define _SIRBUF_MAX 5

#define _SIR_L_START(format) \
    bool    r = false;       \
    va_list args;            \
    va_start(args, format);

#define _SIR_L_END(args) va_end(args);

#define validstr(n) (NULL != n && *n != '\0')

#define validid(id) (id >= 0)

#define validlevels(levels) ((levels & SIRL_ALL) != 0 && (levels & SIRO_MSGONLY) == 0)

#define validopts(opts) ((opts & SIRL_ALL) == 0 && opts <= SIRO_MSGONLY)

#define flagtest(flags, test) ((flags & test) == test)

#define safefree(p) \
    if (p) {        \
        free(p);    \
        p = NULL;   \
    }

#define safefclose(f) \
    if (f) {          \
        fclose(f);    \
        f = NULL;     \
    }

#define resetstr(str) str[0] = (sirchar_t)'\0';

bool _sir_lv(sir_level level, const sirchar_t* format, va_list args);
void _sir_l(const sirchar_t* format, ...);

bool             _sir_dispatch(sir_level level, siroutput* output);
const sirchar_t* _sir_format(sir_options, siroutput* output);
bool             _sir_stderr_write(const sirchar_t* message);
bool             _sir_stdout_write(const sirchar_t* message);
#ifndef SIR_NO_SYSLOG
int _sir_syslog_maplevel(sir_level level);
#endif

void       _sirbuf_reset(sirbuf* buf);
sirchar_t* _sirbuf_get(sirbuf* buf, size_t idx);

sirfile* _sirfile_create(const sirchar_t* path, sir_levels levels, sir_options opts);
bool     _sirfile_write(sirfile* sf, const sirchar_t* output);
bool     _sirfile_writeheader(sirfile* sf);
void     _sirfile_destroy(sirfile* sf);
bool     _sirfile_validate(sirfile* sf);

FILE* _sir_fopen(const sirchar_t* path);
void  _sir_fclose(FILE** f);
void  _sir_fflush(FILE* f);

int  _sir_files_add(sirfiles* sfc, const sirchar_t* path, sir_levels levels, sir_options opts);
bool _sir_files_rem(sirfiles* sfc, int id);
bool _sir_files_destroy(sirfiles* sfc);
bool _sir_files_dispatch(sirfiles* sfc, sir_level level, siroutput* output);

bool _sir_options_sanity(const sirinit* si);

const sirchar_t* _sir_levelstr(sir_level level);
bool             _sir_destwantslevel(sir_levels destLevels, sir_level level);

bool  _sir_getlocaltime(time_t* tbuf, long* nsecbuf);
bool  _sir_formattime(time_t now, sirchar_t* buffer, const sirchar_t* format);

pid_t _sir_getpid();
pid_t _sir_gettid();

#ifdef _WIN32
void _sir_invalidparam(const wchar_t* expression, const wchar_t* function, const wchar_t* file,
    unsigned int line, uintptr_t pReserved);
#endif

/*! \endcond */

#endif /* !_SIR_H_INCLUDED */
