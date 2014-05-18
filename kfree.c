#include <stdio.h>
#include <assert.h>
#include "alloc.h"
#include "io.h"
#include "fd.h"

/* for definitions, see malloc92.c */
extern uintptr_t heapstart;
extern struct header *alloclist; 
extern struct header *freelist;      
extern struct header freeroot;      

int my_errno = 0;

/* checks whether 'node' was allocated previously; if true, removes it from the
 * alloclist and sets was_allocated flag to true
 *
 * returns 1 if 'node' is in the alloclist, 0 otherwise
 */
int was_allocated(struct header *node) {
    struct header *folw, *prev;
    int was_allocated = 0;     /* return flag */
    for (prev = NULL, folw = alloclist; folw; prev = folw, folw = folw->next) 
        if (folw == node) {         /* it's in the used list */
            was_allocated = 1;   /* set return flag */
            (!prev) ? (alloclist = node->next) : (prev->next = node->next);
            break;  /* remove from the list and stop iterating */
        } 
    return was_allocated;
}

/**
 * frees the 'ptr' if it was allocated; if 'ptr' is NULL or wasn't allocated
 * previously by my_malloc just returns
 *
 * returns void
 */
void kfree (void *ptr) {
    if (ptr == NULL) return;    /* senseless to free NULL */
    
    struct header *curr = (struct header *)ptr - 1; /* get ptr to header */

    /* here we are sure that we are given the ptr that was in alloclist */
    if (!was_allocated(curr)) { 
        char fbuf[256];
        sprintf(fbuf, "Error: tried to free memory that wasn't allocated: %p(relative address: %lu)\r\n", ptr, (uintptr_t)ptr - heapstart);
        kprintf(STDOUT, fbuf);
        my_errno = 1;
        return;
    }

    struct header *iter;

    /* find the proper position to insert into ordered freelist */
    for (iter = freelist; !(curr > iter && curr < iter->next); iter = iter->next)
        if (iter >= iter->next && (curr > iter || curr < iter->next)) break;

    if (curr + curr->segs == iter->next) {  /* coalesce next */
        curr->segs += iter->next->segs;     /* add segments */
        curr->next = iter->next->next;      /* adjust next pointer */
    } else
        curr->next = iter->next;            /* insert into the list */

    if (iter + iter->segs == curr) {        /* coalesce prev */
        iter->segs += curr->segs;           /* add segments */
        iter->next = curr->next;            /* adjust prev pointer */
    } else
        iter->next = curr;                  /* insert into the list */

    freelist = iter;                /* point freelist to the last freed block */
    return;
}

/**
 * returns pid field from struct header of the 'ptr' argument
 */
unsigned get_current_pid(void *ptr) {
    struct header *hdr = (struct header *)ptr - 1; /* get ptr to header */
    return hdr->pid;
}
