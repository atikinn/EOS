#include <stdlib.h>
#include "fd.h"
#include "fs.h"
#include "utils.h"
#include "alloc.h"

struct file_desc *
fd_create(struct vnode *vn, int offset) {
    struct file_desc *fd = kmalloc(sizeof *fd);
    if (fd == NULL) return NULL;       /* out of memory */

    fd->vn = vn;
    fd->offset = offset;
    return fd;
}

int
get_fd(void) {
    for (int i = 0; i < OPEN_MAX; i++)
        if (fd_table[i] == NULL) return i;
    return -1;
}

int
fd_destroy(struct file_desc *fd) {
    kfree(fd);
    return 0;
}

