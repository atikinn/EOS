#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "alloc.h"
#include "utils.h"
#include "sdram.h"
#include "lcdc.h"

/* initial heap size */
unsigned initialized;       /* flag to initialize heap only once */
uintptr_t heapstart;        /* pointer to the start of the heap */
struct header *alloclist;   /* unordered singly linked list of nodes in use */
struct header *freelist;    /* ordered by address circular list of free nodes */
struct header freeroot;     /* real root of the freelist */

/* for debugging with valgrind */
void destroy() {
    free(freelist);
}

/**
 * allocated heap and initializes freelist and its root on the stack
 *
 * returns void
 */
static void init (uint32_t size) {
    freelist = (struct header *)(SDRAM_START + LCDC_FRAME_BUFFER_SIZE);
    heapstart = (uintptr_t) freelist;
    freelist->next = &freeroot; 
    freelist->segs = size / sizeof(struct header) - 1;
    //char fbuf[256];
    //sprintf(fbuf, "heapsize = %d\n", freelist->segs);
    //eputs(fbuf, stderr);
    initialized = 1;
    freeroot.next = freelist;
}

/** 
 * finds the next-fit block in the freelist
 *
 * returns the pointer to the data field of the block or NULL if not found
 */
static struct header *find_next_fit(struct header *prev, unsigned seg_num) {
    struct header *curr = prev->next;
    while (1) {  
        if (curr->segs >= seg_num) {        /* found next fit */
            if (curr->segs == seg_num) {    /* exact size match */
                prev->next = curr->next;    /* remove from the list */
            } else {                        /* too big => split the block */
                curr->segs -= seg_num;      /* shrink it for seg size */
                curr += curr->segs;         /* move curr to the ptr to return */
                curr->segs = seg_num;       /* set the seg size */
            }
            freelist = prev;                /* point freelist to the prev ptr */
            curr->pid = 0;                  /* set process pid */
            curr->next = alloclist;         /* add to alloc list */
            alloclist = curr;               /* add to the head of the alloclist */
            return (void *)curr->data;      /* return pointer after the header */
        }
        if (curr == freelist) return NULL;  /* wrapped around the freelist */
        prev = curr, curr = curr->next;     /* advance */
    }
}

/**
 * computes the number of aligned segments required for the 'size' allocation
 *
 * returns the number of segments
 */
static inline unsigned compute_segs(unsigned size) {
    return ((size + sizeof(struct header) - 1) / sizeof (struct header)) + 1;
}

/**
 * allocates block of memory of size 'size'
 *
 * returns ptr to the memory
 */
void *kmalloc (unsigned size) {
    struct header *prev;
    if (size == 0) return NULL; /* senseless to store just a header */

    if (!initialized) {
        init(SDRAM_SIZE - LCDC_FRAME_BUFFER_SIZE); /* first call => initialize freelist */
        prev = &freeroot;   /* set prev to the root */
    } else 
        prev = freelist;    /* set prev to where it was on the prev iteration */

    unsigned seg_num = compute_segs(size);  /* get number of blocks  */
    return find_next_fit(prev, seg_num);
}
