/*
 * smart_util.c
 *
 *  Created on: Jan 15, 2016
 *      Author: TianyuanPan
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "smartac_util.h"
#include "smartac_debug.h"
#include "smartac_common.h"



#define WD_SHELL_PATH "/bin/sh"


/**
 *
 * */
int  execute(const char *cmd_line, int quiet)
{
    int pid, status, rc;

    const char *new_argv[4];
    new_argv[0] = WD_SHELL_PATH;
    new_argv[1] = "-c";
    new_argv[2] = cmd_line;
    new_argv[3] = NULL;

    pid = safe_fork();
    if (pid == 0) {             /* for the child process:         */
        /* We don't want to see any errors if quiet flag is on */
        if (quiet)
            close(2);
        if (execvp(WD_SHELL_PATH, (char *const *)new_argv) == -1) { /* execute the command  */
            debug(LOG_ERR, "execvp(): %s", strerror(errno));
        } else {
            debug(LOG_ERR, "execvp() failed");
        }
        exit(1);
    }

    /* for the parent:      */
    debug(LOG_DEBUG, "Waiting for PID %d to exit", pid);
    rc = waitpid(pid, &status, 0);
    debug(LOG_DEBUG, "Process PID %d exited", rc);

    if (-1 == rc) {
        debug(LOG_ERR, "waitpid() failed (%s)", strerror(errno));
        return 1; /* waitpid failed. */
    }

    if (WIFEXITED(status)) {
        return (WEXITSTATUS(status));
    } else {
        /* If we get here, child did not exit cleanly. Will return non-zero exit code to caller*/
        debug(LOG_DEBUG, "Child may have been killed.");
        return 1;
    }
}




static void _pstr_grow(pstr_t *);

/**
 * Create a new pascal-string like pstr struct and allocate initial buffer.
 * @param None.
 * @return A pointer to an opaque pstr_t string, think java StringBuilder
 */
pstr_t *
pstr_new(void)
{
    pstr_t *new;

    new = (pstr_t *)safe_malloc(sizeof(pstr_t));
    new->len = 0;
    new->size = MAX_BUF;
    new->buf = (char *)safe_malloc(MAX_BUF);

    return new;
}

/**
 * Convert the pstr_t pointer to a char * pointer, freeing the pstr_t in the
 * process. Note that the char * is the buffer of the pstr_t and must be freed
 * by the called.
 * @param pstr A pointer to a pstr_t struct.
 * @return A char * pointer
 */
char *
pstr_to_string(pstr_t *pstr)
{
    char *ret = pstr->buf;
    free(pstr);
    return ret;
}

/**
 * Grow a pstr_t's buffer by MAX_BUF chars in length.
 * Program terminates if realloc() fails.
 * @param pstr A pointer to a pstr_t struct.
 * @return void
 */
static void
_pstr_grow(pstr_t *pstr)
{
    pstr->buf = (char *)safe_realloc((void *)pstr->buf, (pstr->size + MAX_BUF));
    pstr->size += MAX_BUF;
}

/**
 * Append a char string to pstr_t, allocating more memory if needed.
 * If allocation is needed but fails, program terminates.
 * @param pstr A pointer to a pstr_t struct.
 * @param string A pointer to char string to append.
 */
void
pstr_cat(pstr_t *pstr, const char *string)
{
    size_t inlen = strlen(string);
    while ((pstr->len + inlen + 1) > pstr->size) {
        _pstr_grow(pstr);
    }
    strncat((pstr->buf + pstr->len), string, (pstr->size - pstr->len - 1));
    pstr->len += inlen;
}

/**
 * Append a printf-like formatted char string to a pstr_t string.
 * If allocation fails, program terminates.
 * @param pstr A pointer to a pstr_t struct.
 * @param fmt A char string specifying the format.
 * @param ... A va_arg list of argument, like for printf/sprintf.
 * @return int Number of bytes added.
 */
int
pstr_append_sprintf(pstr_t *pstr, const char *fmt, ...)
{
    va_list ap;
    char *str;
    int retval;

    va_start(ap, fmt);
    retval = safe_vasprintf(&str, fmt, ap);
    va_end(ap);

    if (retval >= 0) {
        pstr_cat(pstr, str);
        free(str);
    }

    return retval;
}
