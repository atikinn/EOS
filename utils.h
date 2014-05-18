#ifndef UTILS_H
#define UTILS_H

#define DFL_PROMPT "$ "

#include <stdlib.h>

typedef struct {        /* structure for the input to store and parse */
    char *line;         /* raw input line */
    const char **argv;  /* argument verctor */
    int argc;           /* number of arguments */
} input_t;

/* global enums */
enum g_enums {
    SUCCESS = 0,
    DEC_BASE = 10,
    HEX_BASE = 16,
    LINE_SIZE = 256,
    QUOTE = '"'
};

void panic(void);

char *newstr(const char *arg, size_t len);
void eputs(int fd, const char *str);	/* userland wrapper around svc_write */
void eputc(int fd, const char ch);
char egetc(int fd);
void mmap(void);

int eopen(const char *path);
int eclose(int fd);
int ecreate(const char *path);
int eremove(const char *path);

void *emalloc(size_t n);	/* userland wrapper around svc_malloc */
void efree(void *ptr);		/* userland wrapper around svc_free */
int strtol_wrap(const char *num, long *res, int base);
input_t *input_t_init(size_t size);
void freecmd(input_t *arglist);
int strtoul_wrap(const char *num, unsigned long *res, int base);
int strtoull_wrap(const char *num, unsigned long long *res, int base);
void char2ascii(unsigned char ch, char *string);

int ls(void);
int pwd(void);
int cd(const char *dir);

#endif  /* UTILS_H */
