/**
 * svc.h
 * Routines for supervisor calls
 *
 * ARM-based K70F120M microcontroller board
 *   for educational purposes only
 * CSCI E-92 Spring 2014, Professor James L. Frankel, Harvard Extension School
 *
 * Written by James L. Frankel (frankel@seas.harvard.edu)
 */

#ifndef _SVC_H
#define _SVC_H

#include "time.h"

#define SVC_MaxPriority 15
#define SVC_PriorityShift 4

// Implemented SVC numbers
enum svc_calls {
	SVC_MALLOC 	= 0,
	SVC_FREE 	= 1,
	SVC_OPEN	= 2,
	SVC_CLOSE	= 3,
	SVC_READ	= 4,
	SVC_WRITE	= 5,
	SVC_CREATE	= 6,
	SVC_REMOVE	= 7,
	SVC_MMAP	= 8,
	SVC_CWD		= 9,
	SVC_CD		= 10,
	SVC_LS		= 11,
	SVC_SETTIME = 12,
	SVC_GETTIME = 13,
    SVC_SETPDB 	= 14,
	SVC_PWD		= 15,
};

void svcInit_SetSVCPriority(unsigned char priority);
void svcHandler(void);

void *svc_malloc(unsigned size);
void svc_free(void *ptr);
void svc_mmap(void);

int svc_open(const char *path);
int svc_close(int fd);
int svc_read(int fd, char *byte);
int svc_write(int fd, const char byte);
int svc_create(const char *path);
int svc_remove(const char *path);

void svc_gettime(const struct tval *tval);
void svc_settime(const uint64_t *time);
void svc_setpdb(const long delay, int (*fnc_p)(void));

int svc_ls(void);
int svc_pwd(void);
int svc_cd(const char *path);

#endif /* _SVC_H */
