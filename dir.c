#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fs.h"
#include "dev.h"
#include "utils.h"
#include "dir.h"
#include "file.h"
#include "alloc.h"
#include "fd.h"
#include "io.h"

const struct vfs_ops dir_ops = {
    .open   = NULL,
    .close  = dir_close,
    .write  = dir_write,
    .create = NULL,
    .remove = dir_remove,
    .stat   = dir_list,
    .mkdir  = NULL,
    .rmdir  = NULL,
    .read   = dir_read,
    .readdir = NULL,
    .init   = NULL
};

static
struct vnode *
dir_alloc(enum fs_entry_type type, char *path) {
    struct vnode *vn = vnode_create(type);
    if (vn == NULL) return NULL;

    vn->fs = fs_entry_create(DIRECTORY);
    vn->fs->dir = kmalloc (sizeof *vn->fs->dir);
    if (vn->fs == NULL || vn->fs->dir == NULL) {
        vnode_destroy(vn);
        return NULL;
    }

    memset(vn->fs->dir, 0, sizeof *vn->fs->dir); /*sets paretn to NULL */
    vn->fs->dir->name = path;
    vn->ops = &dir_ops; 
    
    return vn;
}

int
dir_create(struct vnode *pdir, enum fs_entry_type type, char *path) {
    struct vnode *dir = dir_alloc(type, path);
    if (dir == NULL) return -2;

    if (pdir == NULL) {
        assert (cwd == NULL);
        cwd = dir;
        return 0;
    }

    assert(pdir != NULL);
    
    int i;
    for (i = 0; i < DIR_MAX; i++)
        if (pdir->fs->dir->subdir[i] == NULL) break;

    if (i == DIR_MAX) { 
        kprintf(STDOUT, "the directory is full\r\n");
        return -2;
    }

    pdir->fs->dir->subdir[i] = dir;
    pdir->fs->dir->subdir_count++;   
    dir->fs->dir->parent = pdir;
    return 0;
}

int 
dir_open(struct vnode **dir, const char *dir_name) {
    return 0;
}

int 
dir_close(struct vnode *dir){
    return 0;
}

int 
dir_read (struct vnode *vn, char *byte, int offset) {
    return 0;
}

int 
dir_write(struct vnode *vn, char byte) {
    return 0;
}

int 
dir_remove(struct vnode *dir, const char *name) {
    return 0;
}

int 
dir_list(struct vnode *dir) {
    char fbuf[256];

    sprintf(fbuf, "Total files: %d\r\n", dir->fs->dir->file_count);
    kprintf(STDOUT, fbuf);

    for (int i = 0; i < FILE_MAX; i++)
        if (dir->fs->dir->files[i] != NULL) {
            sprintf(fbuf, "\t%s: %-10d\r\n", 
                    dir->fs->dir->files[i]->fs->file->name,
                    dir->fs->dir->files[i]->fs->file->size);
            kprintf(STDOUT, fbuf);
        }

    for (int i = 0; i < DIR_MAX ; i++)
        if (dir->fs->dir->subdir[i] != NULL) {
            sprintf(fbuf, "\t%s: dir\r\n", 
                    dir->fs->dir->subdir[i]->fs->dir->name);
            kprintf(STDOUT, fbuf);
        }
    return 0;
}
