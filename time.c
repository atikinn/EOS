/*
 * time.c
 *
 *  Created on: May 16, 2014
 *      Author: isinyagin
 */

#include "flextimer.h"
#include "time.h"
#include "PDB.h"
#include "command.h"
#include "svc.h"
#include "io.h"
#include "fd.h"

#define MS_IN_SEC 1000

static uint64_t ms = 0;
static uint64_t pdbfire;
static int (*pdbaction) (void);

void flexTimer0Action(void) {
	ms++;
	if (ms == pdbfire) PDB0Start();
}

void PDB0Action(void) {
    svcInit_SetSVCPriority(6);
	//pdbaction();
	kprintf(STDOUT, "pdbtest\r\n");
    svcInit_SetSVCPriority(15);
    PDB0Stop();
}

void getcurrenttime(struct tval *ptr) {
	__asm("cpsid i");
	ptr->tval_sec = ms / MS_IN_SEC;
	ptr->tval_usec = ms - (ptr->tval_sec * MS_IN_SEC);
	__asm("cpsie i");
}

void setcurrenttime(uint64_t *time) {
	__asm("cpsid i");
	ms = *time * MS_IN_SEC;
	__asm("cpsie i");
}

void setpdb(long sec, int (*fnc_p)(void)) {
	__asm("cpsid i");
	pdbfire = ms + (sec * MS_IN_SEC);
	pdbaction = fnc_p;
	__asm("cpsie i");
}
