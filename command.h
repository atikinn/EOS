#ifndef COMMAND_H
#define COMMAND_H

#include "utils.h"
#include "lcdcConsole.h"

#define CHAR_EOF 4

/* cmd_ptr typedef for readability */
typedef int (*cmd_ptr)(int argc, const char *argv[]);

typedef struct {            /* structure for the command object */
    const char *name;       /* name of the command */
    const cmd_ptr fcn_p;    /* pointer to the function */
} command_t;

/* state of the FMS to count number of input arguments */
typedef enum {STRING, BETWEEN, QUOTED} state_t;

input_t *get_cmd(const char *prompt, int prev_rv);
#endif
