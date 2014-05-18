#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "process.h"
#include "date.h"
#include "command.h"
#include "svc.h"
#include "io.h"
#include "alloc.h"
#include "fd.h"
#include "programs.h"

extern int my_errno;

static cmd_ptr builtin_cmd(const char *cmd);
static int cmd_help(int argc, const char *argv[]);
static int cmd_echo(int argc, const char *argv[]);
static int cmd_mmap(int argc, const char *argv[]);
static int cmd_free(int argc, const char *argv[]);
static int cmd_exit(int argc, const char *argv[]);
static int cmd_malloc(int argc, const char *argv[]);
static int cmd_fopen(int argc, const char *argv[]);
static int cmd_fclose(int argc, const char *argv[]);
static int cmd_fputc(int argc, const char *argv[]);
static int cmd_fgetc(int argc, const char *argv[]);
static int cmd_fremove(int argc, const char *argv[]);
static int cmd_fcreate(int argc, const char *argv[]);
static int cmd_ls(int argc, const char *argv[]);
static int cmd_pwd(int argc, const char *argv[]);
static int cmd_mkdir(int argc, const char *argv[]);
static int cmd_cd(int argc, const char *argv[]);
static int cmd_run(int argc, const char *argv[]);

/* build-in commands */
static const command_t commands[] = {
    {"cd",      cmd_cd},
    {"close",   cmd_fclose},
    {"date",    cmd_date},
    {"echo",    cmd_echo},
    {"exit",    cmd_exit},
    {"free",    cmd_free},
    {"help",    cmd_help},
    {"ls",      cmd_ls},
    {"malloc",  cmd_malloc},
    {"mkdir",   cmd_mkdir},
    {"mmap",    cmd_mmap},
    {"open",    cmd_fopen},
    {"pwd",     cmd_pwd},
    {"read",    cmd_fgetc},
    {"rm",      cmd_fremove},
    {"run",     cmd_run},
    {"setdate", cmd_setdate},
    {"touch",   cmd_fcreate},
    {"write",   cmd_fputc},

    { NULL, NULL }
};

static const char *samples[] = { 
		"ser2lcd", 
		"touch2led", 
		"ptn2ser", 
		"thrm2ser", 
		"pb2led",
};

int
do_error(int rv) {
	char buf[265];
	switch(rv) {
		case SUCCESS:
			return rv;
		case EARGNUM:
			sprintf(buf, "wrong number of arguments\r\n");
			break;
		case EEXITST:
			sprintf(buf, "wrong exit value\r\n");
			break;
		case EPARSE:
			sprintf(buf, "parse error\r\n");
			break;
		case ENOMEMR:
			sprintf(buf, "not enough memory\r\n");
			break;
		case EBADMA:
			sprintf(buf, "memory addr is out of bounds\r\n");
			break;
		case EINVADDR:
            sprintf(buf, "memory addr is invalid (!%% 8)\r\n");
			break;
		case EINVPATH: 
            sprintf(buf, "path is not valid\r\n");
			break;
		case ENTFND: 
            sprintf(buf, "file or directory not found\r\n");
			break;
		case ENOCWD: 
            sprintf(buf, "no current working directory\r\n");
			break;
		default:
            sprintf(buf, "unknown error\r\n");
			break;
	}
    eputs(STDOUT, buf);
    return rv;
}

/*
 * executes the command (args[0])
 * 
 * args - arguments vector
 *
 * returns the exit status of the called command or CMDNFND if no command was
 * found
 */
int 
process(const input_t *args) {       
    cmd_ptr exec;
    if ((exec = builtin_cmd(args->argv[0])))
        return do_error(exec(args->argc, args->argv));
    return CMDNFND;   /* command not found */ 
}    

/*
 * checks that command is in the commands array and calls it
 * 
 * cmd - command name
 *
 * returns the exit status of the called command or NULL if no found
 */
static 
cmd_ptr 
builtin_cmd(const char *cmd) {
    int cm;
    for (cm = 0; commands[cm].name; cm++) {
        if (strcmp(cmd, commands[cm].name) < 0) 
            return NULL; /* do not iterate the whole list */
        if (!strcmp(cmd, commands[cm].name))
            return commands[cm].fcn_p;
    }
    return NULL;    /* command not found */
}

/*
 * outputs the functions that shell can perform
 * 
 * argc - number of arguments
 * argv - argument vector
 *
 * returns SUCCESS on success, error code otherwise
 */
static
int 
cmd_help(int argc, const char *argv[]) {
    if (argc != 1) return EARGNUM;

    eputs(STDOUT, "The shell knows only the following commands:\r\n");

    int cm;
    char fbuf[256];
    for (cm = 0; commands[cm].name; cm++) {
        sprintf(fbuf, "\t#%d: %s\r\n", cm+1, commands[cm].name);
        eputs(STDOUT, fbuf);
    }
    return SUCCESS;
}

/*
 * echo argv[1-...] to the stdout
 * 
 * argc - number of arguments
 * argv - argument vector
 *
 * returns SUCCESS on success
 */
static
int 
cmd_echo(int argc, const char *argv[]) {
    int rv = SUCCESS;

    if (argc == 1) {
        eputs(STDOUT, "\r\n");
        return rv;
    }

    int ac;
    char fbuf[256];
    for (ac = 1; ac < argc-1; ac++) {
        sprintf(fbuf, "%s ", argv[ac]);
        eputs(STDOUT, fbuf);
    }
    sprintf(fbuf, "%s\r\n", argv[argc-1]);
    eputs(STDOUT, fbuf);
   
    /*
    if (ferror(stdout)) { 
        sprintf(fbuf, "Error while printing to stdout: %s\r\n", strerror(errno));
        eputs(STDOUT, fbuf);
        errno = 0;
        rv = EPRINT;
    } 
    */
    return rv;
}

/*
 * exits the shell with a specified status code (0 by default)
 * 
 * argc - number of arguments
 * argv - argument vector
 *
 * exits shell
 */
static
int 
cmd_exit(int argc, const char *argv[]) {
    if (argc == 1) exit(EXIT_SUCCESS);   /* simple exit */

    if (argc > 2) return EARGNUM;
    

    char fbuf[256];
    long exit_status; 
    if (strtol_wrap(argv[1], &exit_status, DEC_BASE) < 0)
        return EPARSE;

    if (exit_status < CHAR_MIN || exit_status > CHAR_MAX) {
        sprintf(fbuf, "Undefined exit status: %ld\r\n", exit_status);
        eputs(STDOUT, fbuf);
        return EEXITST;
    }

    exit(exit_status);  /* save to pass long, checking is done above */
}

static 
long 
convert_bytes(const char *str_bytes) {
    long nbytes;

    int base = (str_bytes[0] == '0' && str_bytes[1] == 'x') ? HEX_BASE : DEC_BASE;
    if (strtol_wrap(str_bytes, &nbytes, base) < 0)
        return EPARSE;

    if (nbytes < 0 || nbytes > 0xffffff) return EBADMA;

    return nbytes;
}

static
int
cmd_malloc(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;
    
    long nbytes = convert_bytes(argv[1]);  /* save to pass long */
    if (nbytes == EBADMA || nbytes == EPARSE) return EPARSE;

    void *rv = emalloc(nbytes * sizeof (char));
    char fbuf[256];
    sprintf(fbuf, "%p\r\n", rv);
    eputs(STDOUT, fbuf);

    return SUCCESS;
}

static 
unsigned long 
convert_addr(const char *addr) {
    unsigned long mem_addr;

    int base = (addr[0] == '0' && addr[1] == 'x') ? HEX_BASE : DEC_BASE;
    if (strtoul_wrap(addr, &mem_addr, base) < 0)
        return EPARSE;

    if (mem_addr < 0 || mem_addr > 0xffffffff) return EBADMA;
    
    if (mem_addr % 8 != 0) return EINVADDR;
    
    return mem_addr;
}

static 
int 
cmd_free(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;

    long mem_addr = convert_addr(argv[1]);
    if (mem_addr == EBADMA || mem_addr == EPARSE) return EPARSE;

    efree((void *)mem_addr);
    if (!my_errno) {
        char fbuf[256];
        sprintf(fbuf, "memory at address %p was freed\r\n", (void *)mem_addr);
        eputs(STDOUT, fbuf);
    }
    return 0;
}

static 
int 
cmd_mmap(int argc, const char *argv[]) {
    if (argc != 1) return EARGNUM;

    svc_mmap();
    return 0;
}

static
int
is_valid_path(const char *path) {
    char cset[] = LEGAL_CHAR_SET;
    size_t length = strlen(path);
    size_t l = strspn(path, cset);
    if (length == l)
        return 1;

    if (length-1 == l && *(path+4) == ':' && strstr(path, "dev") == path)
        return 1;
        
    return 0;
}

static 
int 
cmd_fopen(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;
    
    if (!is_valid_path(argv[1])) return EINVPATH;

    int rv = svc_open(argv[1]);
    if (rv < 0) return -3;    /* file doesn't exist */

    char fbuf[256];
    sprintf(fbuf, "opened fd %d for file %s\r\n", rv, argv[1]);
    eputs(STDOUT, fbuf);
    return 0;
}

static 
int 
cmd_fclose(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;

    long fd; 
    if (strtol_wrap(argv[1], &fd, DEC_BASE) < 0) return EPARSE;
    
    return svc_close(fd);
}

static 
int 
cmd_fputc(int argc, const char *argv[]) {
    if (argc != 3) return EARGNUM;

    long fd; 
    if (strlen(argv[2]) != 1 || strtol_wrap(argv[1], &fd, DEC_BASE) < 0)
        return EPARSE;
    
    char byte = *argv[2] - '0';
    return svc_write(fd, byte);
}

static 
int 
cmd_fgetc(int argc, const char *argv[]) {
    if (argc > 3) return EARGNUM;
    // check for the third argument

    long fd; 
    if (strtol_wrap(argv[1], &fd, DEC_BASE) < 0) return EPARSE;
    
    int as_int = 0;
    if (argv[2] ) { 
        if (strcmp(argv[2], "int") != 0)return EPARSE;
        as_int = 1;
    }

    char byte;
    char fbuf[256];
    if (svc_read(fd, &byte) == -2)
        eputs(STDOUT, "end of file\r\n");
    else if (as_int){
        sprintf(fbuf, "%c\r\n", (int)byte);
        eputs(STDOUT, fbuf);
    } else {
        sprintf(fbuf, "%c\r\n", byte + '0');
        eputs(STDOUT, fbuf);
    }

    return 0; 
}

static 
int 
cmd_fremove(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;

    if (!is_valid_path(argv[1])) return EINVPATH;
    
    if (svc_remove(argv[1]) == -1) return ENTFND;

    return 0;
}

static 
int 
cmd_fcreate(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;
    
    if (!is_valid_path(argv[1])) return EINVPATH;

    if (svc_create(argv[1]) == -1) return -3;

    return 0;
}

static 
int 
cmd_ls(int argc, const char *argv[]) {
    if (argc != 1) return EARGNUM;
    //if (ls() != 0) return -1;
    ls();
    return 0;
}

static 
int 
cmd_pwd (int argc, const char *argv[]) {
    if (argc != 1) return EARGNUM;
    if (pwd() == -1) return ENOCWD;
    
    return 0;
}

static 
int 
cmd_mkdir(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;
    
    // TODO: better error checking
    if (is_valid_path(argv[1]) && mkdir(argv[1])) return EARGNUM;

    return 0;
}

static 
int 
cmd_cd(int argc, const char *argv[]) {
    if (argc != 2) return EARGNUM;
    
    if (is_valid_path(argv[1]) && cd(argv[1])) return EARGNUM;

    return 0;
}

static 
int 
cmd_run(int argc, const char *argv[]) {
    if (argc > 3) return EARGNUM;

    int i = -1;
    for (i = 0; samples[i]; i++)
    	if (!strcmp(argv[1], samples[i]))
    		break;

    int (*fptr)(void);
    switch(i) {
    	case 0: fptr = &ser2lcd; break;
    	case 1: fptr = &touch2led; break;
    	case 2: fptr = &ptn2ser; break;
    	case 3: fptr = &thrm2ser; break;
    	case 4: fptr = &pb2led; break;
    	default:
    		eputs(STDOUT, "No such sample program\r\n");
    		return -1;
    }
    
    long delay;
    if (argv[2]) {
        if (strtol_wrap(argv[2], &delay, DEC_BASE) < 0)
            return EPARSE;
        svc_setpdb(delay, fptr);
        return 0;
    }

    return fptr();
}
