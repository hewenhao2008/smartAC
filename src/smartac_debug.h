/*
 * smartac_debug.h
 *
 *  Created on: Jan 16, 2016
 *      Author: TianyuanPan
 */

#ifndef SMARTAC_DEBUG_H_
#define SMARTAC_DEBUG_H_

typedef struct _debug_conf {
    int debuglevel;      /**< @brief Debug information verbosity */
    int log_stderr;      /**< @brief Output log to stdout */
    int log_syslog;      /**< @brief Output log to syslog */
    int syslog_facility; /**< @brief facility to use when using syslog for logging */
} debugconf_t;

extern debugconf_t debugconf;

/** Used to output messages.
 * The messages will include the filename and line number, and will be sent to syslog if so configured in the config file
 * @param level Debug level
 * @param format... sprintf like format string
 */
#define debug(level, format...) _debug(__FILE__, __LINE__, level, format)

/** @internal */
void _debug(const char *, int, int, const char *, ...);

#endif /* SMARTAC_DEBUG_H_ */
