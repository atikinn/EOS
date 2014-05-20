#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "command.h"
#include "parse.h"
#include "process.h"
#include "utils.h"
#include "fs_boot.h"
#include "derivative.h"
#include "fd.h"
#include "svc.h"
#include "PCB.h"

//TODO: environment
//TODO: escapes

/*
 * main loop of the shell: gets command, parses, processes, checks return
 * value, frees data
 *
 * argc - right now doesn't do anything
 * argv - right now doesn't do anything
 *
 * exits EXIT_SUCCESS unless any error occurs
 */

int interactive (int argc, char *argv[]) {
	(void)argc;
	(void)argv;
    char *prompt = DFL_PROMPT;
    int rv = 0;
    input_t *cmd;
    while ((cmd = get_cmd(prompt, rv))) {
        if ((cmd->argv = parseline(cmd)))
            if ((rv = process(cmd)) == CMDNFND) {
            	char fbuf[256];
                sprintf(fbuf, "%s: command not found\r\n", cmd->argv[0]);
                eputs(STDOUT, fbuf);
            }
        freecmd(cmd);
    }

    return 0;
}

int set_buffer(void) {
     if(setvbuf(stdin, NULL, _IONBF, 0)) {
    	 printf("setvbuf failed on stdin: %s", strerror(errno));
    	 return -1;
     }
     
     if(setvbuf(stdout, NULL, _IONBF, 0)) {
    	 printf("setvbuf failed on stdout: %s", strerror(errno));
    	 return -1;
     }
     return 0;
}

void privUnprivileged(void) {
	__asm(
		"mrs r0,CONTROL"			"\n"
		"orr r0,r0,#1"				"\n"
		"msr CONTROL,r0"			"\n"
		"isb sy"
	);
}

/* to silent the linker with c9x Librarian */
int ReadUARTN(void* bytes, unsigned long length ) {
	(void)bytes;
	(void)length;
	return 0;
}

/* to silent the linker with c9x Librarian */
int WriteUARTN(void* bytes, unsigned long length) {
	(void)bytes;
	(void)length;
	return 0;
}

/* to silent the linker with c9x Librarian */
int InitializeUART( int baudrate ) {
	(void)baudrate;
	return 0;
}

//TODO: setdate right now only accepts timestamps (in seconds)
//TODO: multiprocessing

int shell_init() {
    //kfork(&interactive);
	return interactive(0, NULL);
    //return 0;
}

int main(int argc, char *argv[]) {
	if (set_buffer() == -1) return 1;
    if (boot() == -1) return 1;	/* other devices are also booted here */
    svcInit_SetSVCPriority(15);
    pcbq_init();

    privUnprivileged();
    return shell_init();
}
