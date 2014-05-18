#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h> 
#include <errno.h>
#include <assert.h>
#include <arith.h>
#include "utils.h"
#include "svc.h"
#include "alloc.h"
#include "fd.h"

/*
 * wrapper function around strtol call; converts and checks for error
 *
 * num - string to convert
 * res - storage location of the converted value
 *
 * returns SUCCESS on success and < 0 on error
 */
int 
strtol_wrap(const char *num, long *res, int base) {                                        
    char fbuf[256];

    if (num == NULL) {
        sprintf(fbuf, "Parse error: %s\r\n", "passed NULL pointer");
        eputs(STDOUT, fbuf);
        return -3;
    }

    int rv = SUCCESS;

    char *end = NULL;
    *res = strtol(num, &end, base);

    if (errno) {    /* value out of range: under or over flow */
        sprintf(fbuf, "Parse error: %s\r\n", strerror(errno));
        eputs(STDOUT, fbuf);
        errno = 0;
        rv = -1;
    }

    if (*end) {
        sprintf(fbuf, "Parse error: non-parsing part: %s\r\n", end);
        eputs(STDOUT, fbuf);
        rv = -2;
    }

    return rv;                                         
}  

int 
strtoul_wrap(const char *num, unsigned long *res, int base) {                                        
    char fbuf[256];

    if (num == NULL) {
        sprintf(fbuf, "Parse error: %s\r\n", "passed NULL pointer");
        eputs(STDOUT, fbuf);
        return -3;
    }

    int rv = SUCCESS;

    char *end = NULL;
    *res = strtoul(num, &end, base);

    if (errno) {    /* value out of range: under or over flow */
        sprintf(fbuf, "Parse error: %s\r\n", strerror(errno));
        eputs(STDOUT, fbuf);
        errno = 0;
        rv = -1;
    }

    if (*end) {
        sprintf(fbuf, "Parse error: non-parsing part: %s\r\n", end);
        eputs(STDOUT, fbuf);
        rv = -2;
    }

    return rv;                                         
}  

int 
strtoull_wrap(const char *num, unsigned long long *res, int base) { 
    char fbuf[256];

    if (num == NULL) {
        sprintf(fbuf, "Parse error: %s\r\n", "passed NULL pointer");
        eputs(STDOUT, fbuf);
        return -3;
    }

    int rv = SUCCESS;

    char *end = NULL;
    *res = strtoull(num, &end, base);

    if (errno) {    /* value out of range: under or over flow */
        sprintf(fbuf, "Parse error: %s\r\n", strerror(errno));
        eputs(STDOUT, fbuf);
        errno = 0;
        rv = -1;
    }

    if (*end) {
        sprintf(fbuf, "Parse error: non-parsing part: %s\r\n", end);
        eputs(STDOUT, fbuf);
        rv = -2;
    }

    return rv;                                         
}  

/* 
 * almost a replica of strdup, didn't know whether we are allowed to use it
 * (it's not in the standard, but part of POSIX)
 *
 * arg_str - string to duplicate
 * len - length of the string
 *
 * returns pointer to the newly allocated string
 */
char 
*newstr(const char *arg_str, size_t len) {
    char *rv = emalloc(sizeof (char) * (len+1));
    if (rv == NULL) return NULL;
    strncpy(rv, arg_str, len);
    rv[len] = '\0';
    return rv;
}

/*
 * malloc wrapper which checks for errors and fails immideately
 *
 * n - size to allocate
 *
 * returns void
 */
void 
*emalloc(size_t n) {
    void *rv;
    if ((rv = svc_malloc(n)) == NULL)
        eputs(STDOUT, "out of memory");
    return rv;
}

void
eputs(int fd, const char *str) {
	while(*str) svc_write(fd, *str++);
}

void 
eputc(int fd, const char ch) {
	svc_write(fd, ch);
}

char 
egetc(int fd) {
	char ch;
	svc_read(fd, &ch);
	return ch;
}

void mmap(void) {
	svc_mmap();
}

void 
efree(void *ptr) {
    svc_free(ptr);
}

int 
eopen(const char *path) {
	return svc_open(path);
}

int 
eclose(int fd) {
	return svc_close(fd);
}

int 
ecreate(const char *path) {
	return svc_create(path);
}

int 
eremove(const char *path) {
	return svc_remove(path);
}

/* 
 * initializes input_t object with size string to read input
 *
 * returns pointer to the allocated object
 */
input_t* 
input_t_init(size_t size) {
    input_t *buf = emalloc(sizeof *buf);
    buf->line = emalloc(sizeof (char) * size + 1);
    buf->argv = NULL;
    buf->argc = 0;
    return buf;
}

/*
 * frees memory of the input_t object
 *
 * returns void
 */
void 
freecmd(input_t *cmd_list) {
    const char **argv_ptr = cmd_list->argv;
    while(*argv_ptr)
        efree((void *)*argv_ptr++);
    efree(cmd_list->argv);
    //efree(cmd_list->line);    /* freed in parseline */
    efree(cmd_list);
}

void
panic(void) {
  __asm("bkpt");
}

int ls(void) {
	return svc_ls();
}

int cd(const char *dir) {
	return svc_cd(dir);
}

int pwd(void) {
	return svc_pwd();
}

void 
char2ascii(unsigned char ch, char *string) {
  string[0] = (char) (ch/100 + '0');
  ch -= ch/100 * 100;
  string[1] = (char) (ch/10 + '0');
  ch -= ch/10 * 10;
  string[2] = (char) (ch + '0');
}
