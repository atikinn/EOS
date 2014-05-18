#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "parse.h"
#include "alloc.h"

/*
 * parses the line and populates argv field of the input_t object with
 * arguments from the command line
 *
 * input_line - object which contains input line
 *
 * returns pointer to allocated argv arrray
 */
const char **parseline(const input_t *input_line) {
    if (input_line->line == NULL) return NULL;   /* no input */
    const char **argv = emalloc(sizeof (char *) * (input_line->argc + 1));
    char *line_ptr = input_line->line;          /* ptr iterator */
    int argc = 0;                               /* num of args */
    while (*line_ptr) {                         /* loop */
        while (is_delim(*line_ptr)) line_ptr++; /* skip white space */
        if (*line_ptr == '\0') break;           /* end of line */
        size_t arg_len = 1;                     /* track the arg len */
        char *start_ptr = line_ptr;             /* first character of the arg */
        if (*line_ptr == QUOTE) {               /* quoted string */ 
            arg_len = 0;                        /* skip first quote */
            start_ptr++;                        /* skip first quote */
            while (*++line_ptr && *line_ptr != QUOTE) arg_len++;
            if (*line_ptr == QUOTE) ++line_ptr; /* closing quote */
        } else                                  /* not quoted, count length */
            while (*++line_ptr && !(is_delim(*line_ptr)) ) arg_len++;
        argv[argc++] = newstr(start_ptr, arg_len); /* store next argument */
    }
    //assert (argc == input_line->argc);          /* double checking */
    argv[argc] = NULL;                          /* terminate argv array */
    efree(input_line->line);                    /* free the bufline */
    return argv;
}
