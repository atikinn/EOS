#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <stdint.h>

//typedef unsigned int uintptr_t;

struct header {
    struct header *next;       /* not portable: assumes that ptr is 4 bytes */
    uint32_t 
        segs: 24,              /* number of blocks including the header, max 128mb of memory) */
        pid: 8;                /* number of pids, support 256 right now  */
    uint8_t data[0];           /* actual data */
};

void *kmalloc (unsigned size);
void kfree (void *ptr);
void kmmap(void);
void destroy(); /* for debugging */

//#define NULL (void *)0

#endif /* _ALLOC_H */
