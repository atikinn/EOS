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

/**
 * prints the contents of one block to stdout in ascii
 *
 * returns the number of segments this block occupies
 */
static unsigned do_print(struct header *iter) {
    unsigned addr = (uintptr_t)iter - heapstart;
    int next_addr = (uintptr_t)iter->next - heapstart;
    char fbuf[256];
    sprintf(fbuf, "--------------------\r\n"
                    "| uhex:  %p    -->\r\n"
                    "| addr: %5d      |\r\n"
                    "| size: %5u      |\r\n"
                    "| next: %5d      |\r\n"
                    "| pid : %5d      |\r\n"
                    "------------------\r\n",
            iter->data, addr, iter->segs * sizeof *iter,
            (iter->next == NULL) ? 0 : (next_addr < 0) ?  -1 : next_addr, 
            iter->pid);
    kprintf(STDOUT, fbuf);
    return iter->segs;
}

/**
 * iterates over a singly alloclist and calls do_print to print each block and
 * computes the sum of all segments' size in 'num_seg'
 *
 * returns 'num_seg' (sum of all segments' size)
 */
static unsigned print_list(struct header *list) {
    struct header *iter;
    unsigned num_segs = 0;
    for (iter = list; iter; iter = iter->next) 
        num_segs += do_print(iter);
    return num_segs;
}

/**
 * iterates over a cyclic freelist and calls do_print to print each block and
 * computes the sum of all segments' size in 'num_seg'
 *
 * returns 'num_seg' (sum of all segments' size)
 */
static unsigned print_cyclic(struct header *head) {
    unsigned num_segs = 0;
    struct header *iter = head;
    char fbuf[256];
    do {
        sprintf(fbuf, "iter: %p\r\n", iter);
        kprintf(STDOUT, fbuf);
        num_segs += do_print(iter);
        iter = iter->next;
    } while (iter != &freeroot);

    return num_segs;
}

/**
 * prints two linked lists: list of free block and of allocated blocks
 *
 * retruns void
 */
void kmmap() {
    unsigned total_segs = 0;
    kprintf(STDOUT, "ALLOCLIST\r\n");
    if ((total_segs += print_list(alloclist)) == 0)
    	kprintf(STDOUT, "\tEmpty");
    char fbuf[256];
    sprintf(fbuf, "Totally allocated: %d\r\n", total_segs * sizeof (struct header));
    kprintf(STDOUT, fbuf);

    kprintf(STDOUT, "\r\nFREELIST\r\n");
    if (!freeroot.next) 
    	kprintf(STDOUT, "\tEmpty");
    else 
        total_segs += print_cyclic(freeroot.next);
    kprintf(STDOUT, "\r\n\r\n");
    //assert (total_segs == HEAPSIZE/sizeof (struct header));
}
