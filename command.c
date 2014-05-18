#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "command.h"
#include "alloc.h"
#include "delay.h"
#include "lcdcConsole.h"
#include "fd.h"

static void print_prompt(const char *prompt, int prev_rv);
static state_t update_argc(int c, state_t arg_state, int *argc);
static input_t * check_input_errors(int c, int pos, input_t *buf);

/* public functions */

/*
 * reads stdin by single character allocating an input_t command object
 *
 * prompt - pointer to the prompt character
 * prev_rv = exit status of the previous command to output
 *
 * returns pointer to input_t object or NULL
 */
input_t *get_cmd(const char *prompt, int prev_rv) {

	eputs(STDOUT, "\r\n");
    print_prompt(prompt, prev_rv);          /* print prompt with exit code */

    input_t *buf = input_t_init(LINE_SIZE);  /* allocate buffer object */
    state_t arg_state = BETWEEN;            /* initial state */
    int c, pos = 0;                         /* input char and position */
    while((c = egetc(STDIN)) != CHAR_EOF && pos < LINE_SIZE) {    /* loop */
        if (c == '\r') {	// if using unix tty, use \n
            //if (buf->line[pos-1] == '\r') pos--; this logic is needed only for debug i/o
            eputs(STDOUT, "\r\n");
            break;
        }

        if (isprint(c) || isspace(c)) {
            eputc(STDOUT, c);	/* output to the Serial port */
            buf->line[pos++] = c;               /* store c */
            arg_state = update_argc(c, arg_state, &buf->argc);  /* count argc */
        }
    }

    return check_input_errors(c, pos, buf); /* check errors */
}

/*
 * checks input data for errors
 *
 * c - last read character
 * pos - last position in the buf
 * buf - pointer to the input_t object
 *
 * returns buf or NULL on error
 */
static 
input_t *
check_input_errors(int c, int pos, input_t *buf) {
    if (c == EOF && pos == 0) {  /* EOF and no imput => exit */
        efree(buf);
        return NULL;
    }

    char fbuf[256];
    /*
    if (ferror(stdin)) {
        sprintf(fbuf, "Error reading string: %s\r\n", strerror(errno));
        eputs(STDOUT, fbuf);
        errno = 0;
        goto cleanup;
    } 
	*/
    if (pos >= LINE_SIZE) {
        sprintf(fbuf, "string exceeded allowable length of %d\r\n", LINE_SIZE);
        eputs(STDOUT, fbuf);
        goto cleanup;
    }

    buf->line[pos] = '\0';
    return buf;

cleanup:
    efree(buf->line);
    buf->line = NULL;   /* indicate no input for parse_line */
    return buf;
}

/*
 * outputs a promtt with exit code of the previous command exit status wasn't 0
 *
 * prompt - prompt character
 * prev_rv - previous exit status
 *
 * returns void
 */
static 
void 
print_prompt(const char *prompt, int prev_rv) {
    char fbuf[256];
    if (prev_rv != 0) {
        sprintf(fbuf, "%s(%d) ", prompt, prev_rv);
        eputs(STDOUT, fbuf);
    } else {
        sprintf(fbuf, "%s ", prompt);
        eputs(STDOUT, fbuf);
    }
}

/*
 * counts the number of arguments on the command line
 *
 * c - current input character
 * arg_state - one of the state_t values
 * argc - counter to increment
 *
 * returns the next state_t of FSM
 */
static 
state_t 
update_argc(int c, state_t arg_state, int *argc) {
    switch (arg_state) {
        case BETWEEN:
            if (c == QUOTE) { 
                arg_state = QUOTED;
                (*argc)++;
            } else if ((isalnum(c) || ispunct(c))) { 
                arg_state = STRING;
                (*argc)++;
            }
            break;
        case STRING:
            if (isspace(c)) arg_state = BETWEEN;
            break;
        case QUOTED:
            if (c == QUOTE) arg_state = BETWEEN;
            break;
    }
    return arg_state;
}
