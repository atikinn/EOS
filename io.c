#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fs.h"
#include "dev.h"
#include "dir.h"
#include "fd.h"
#include "file.h"
#include "io.h"
#include "private.h"
#include "utils.h"
#include "alloc.h"

char *
kstrdup(const char *str) {
    char *ret = kmalloc(strlen(str) + 1);
    if (ret == NULL) return NULL;
    strcpy(ret, str);
    return ret;
}

void kprintf(int fd, const char *str) {
	while(*str)
		kwrite(fd, *str++);
}

int kwrite(int fd, char byte) {
    char fbuf[256];
    if (fd_table[fd] == NULL) { 
        sprintf(fbuf, "no such fd: %zu\r\n", fd);
        kprintf(STDOUT, fbuf);
        return -2;
    }
    int rv = fd_table[fd]->vn->ops->write(fd_table[fd]->vn, byte);
    if (rv == 0 && fd_table[fd]->vn->fs->e_type == FS) fd_table[fd]->offset++;
    return rv;
}

int kread(int fd, char *byte) {
    char fbuf[256];
    if (fd_table[fd] == NULL) { 
        sprintf(fbuf, "no such fd: %d\r\n", fd);
        kprintf(STDOUT, fbuf);
        return -2;
    }

    int rv = fd_table[fd]->vn->ops->read(fd_table[fd]->vn, 
    									 byte, 
                                         fd_table[fd]->offset);
    if (rv == 0 && fd_table[fd]->vn->fs->e_type == FS) fd_table[fd]->offset++;
    return rv;
}

int kremove(const char *path) {
    if (is_dev(path)) return -1; /* devices can be only opened right now */
    return file_remove(cwd, path);
}

int kcreate(const char *path) {
    if (is_dev(path)) return -1; /* devices can be only opened right now */
    char *fname = kstrdup(path);
    if (fname == NULL) return -1; /* ENOMEM */
    return file_create(cwd, REG, fname);
}

int mkdir (const char *path) {
    if (is_dev(path)) return -1; /* devices can be only opened right now */
    char *dirname = kstrdup(path);
    if (dirname == NULL) return -1; /* ENOMEM */
    return dir_create(cwd, REG, dirname);
}

int rmdir (const char *dirname) {
    return 0;
}

int kopen(const char *path) {
    struct vnode *vn = NULL;
    int dev = is_dev(path);
    int offset = dev ? NONSEEK : 0;

    int next = get_fd(); 
    if (next == -1) return -2;  /* fd_table is full, can't open anything  */

    const char *name = (dev) ? path + 4 : path;

    int err = (dev) ? dev_open(&vn, name) : file_open(&vn, name);
    if (err) return -2; /* no such device or file */
    
    fd_table[next] = fd_create(vn, offset);
    if (fd_table[next] == NULL) return -2;  /* ENOMEM */

    /* logically this should be done in file_open, dev_open, dir_open, but
     * complicates error handling */
    vn->ref_count++; 
    return next;
}

int kclose(int fd) {
    char fbuf[256];
    if (fd_table[fd] == NULL) { 
        sprintf(fbuf, "fd #%d is not opened\r\n", fd);
        kprintf(STDOUT, fbuf);
        return -2;
    }

    fd_table[fd]->vn->ops->close(fd_table[fd]->vn);
    fd_destroy(fd_table[fd]);
    fd_table[fd] = NULL;
    return 0;
}

int kls(void) {
    return cwd->ops->stat(cwd);    
}

int kpwd(void) {
    if (cwd == NULL) return -1;
    char fbuf[256];
    sprintf(fbuf, "%s\r\n", cwd->fs->dir->name);
    kprintf(STDOUT, fbuf);
    return 0;
}

int kcd(const char *dir) {
    if (!strcmp("..", dir)) {
        if (!strcmp("/", cwd->fs->dir->name)) {
            kprintf(STDOUT, "already at root\r\n");
        } else {
            cwd = cwd->fs->dir->parent;
        }
        return 0;
    }

    struct vnode *vn = dir_sub_lookup(cwd, dir);
    if (vn == NULL) {
        kprintf(STDOUT, "no such sub directory\r\n");
        return -1;
    }
    
    cwd = vn;
    return 0;
}
