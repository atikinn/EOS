#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fs.h"
#include "utils.h"
#include "file.h"
#include "dir.h"
#include "io.h"
#include "fd.h"
#include "alloc.h"

const struct vfs_ops file_ops = {
    .open   = NULL,
    .close  = file_close,
    .write  = file_write,
    .create = NULL,
    .remove = file_remove,
    .stat   = file_stat,
    .mkdir  = NULL,
    .rmdir  = NULL,
    .read   = file_read,
    .readdir = NULL,
    .init   = NULL
};

static
int
extend_file(struct file *file) {
    if (file->nblocks == 0) {
        struct file_block *fb = kmalloc(sizeof *fb);
        if (fb == NULL) return -1;
        fb->block_size = 0;
        file->data[file->nblocks++] = fb;
    } else {
        unsigned block_size = file->data[file->nblocks-1]->block_size;
        if (block_size == BLOCK_CAPACITY && file->nblocks < FILE_CAPACITY)  {
            struct file_block *fb = (struct file_block *)kmalloc(sizeof *fb);
            if (fb == NULL) return -1;
            fb->block_size = 0;
            file->data[file->nblocks++] = fb;
        }
    } 
    return 0;
}

static
int
release_file_data(struct file *file) {
    for (int i = 0; i < file->nblocks; i++)
        kfree(file->data[i]);
    kfree(file->name);
    kfree(file);
    return 0;
}

int 
file_create(struct vnode *dir, enum fs_entry_type type, char *name) {
    unsigned index = -1; 
    struct vnode *file = dir_lookup(dir, name, &index); /* index unused here */
    char fbuf[256];
    if (file != NULL) { 
        sprintf(fbuf, "file %s already exists\r\n", name);
        eputs(STDERR, fbuf);
        return -1; /*such file or dir already exists */
    }

    for (index = 0; index < FILE_MAX; index++)  /* reuse index var */
        if (dir->fs->dir->files[index] == NULL) break;

    if (index == FILE_MAX) {
        sprintf(fbuf, "%s is full, create a subdir\r\n", dir->fs->dir->name);
        kprintf(STDOUT, fbuf);
        return -1;
    }

    file = vnode_create(FS);
    if (file == NULL) return -1; /* ENOMEM  */
    
    file->fs = fs_entry_create(type);
    file->fs->file = (struct file *)kmalloc(sizeof *file->fs->file);   /*allocate file */
    if (file->fs == NULL || file->fs->file == NULL) {
        vnode_destroy(file);    /*destroys the entry if it isn't NULL */
        return -1; 
    }
    memset(file->fs->file, 0, sizeof *file->fs->file);

    file->fs->file->name = name;
    dir->fs->dir->files[index] = file;
    dir->fs->dir->file_count++;

    file->ops = &file_ops;
    return 0;
};

int 
file_open(struct vnode **file, const char *file_name) {
    unsigned index;
    struct vnode *vn = dir_lookup(cwd, file_name, &index);
    (void)index;
    char fbuf[256];
    if (file == NULL) {
        sprintf(fbuf, "file doesn't exist: %s\r\n", file_name);
        kprintf(STDOUT, fbuf);
        return -1; /* no such file */ 
    }
    *file = vn;
    return 0;
}

int 
file_close(struct vnode *file){
    file->ref_count--;  /* decrement vnode count  */
    return 0;
}

int 
file_write (struct vnode *vn, char byte) {
    struct file *file = vn->fs->file;
    if (extend_file(file) == -1) return -2;
    unsigned nblocks = file->nblocks;
    unsigned block_index = file->data[nblocks-1]->block_size++;
    file->data[nblocks-1]->block[block_index] = byte;
    file->size++;
    return 0;
}

int 
file_read (struct vnode *vn, char *byte, int offset) {
    if (vn->fs->file->size == offset) return -2; /* EOF */

    unsigned block_num = offset / BLOCK_CAPACITY;
    if (vn->fs->file->nblocks <= block_num) return -2;

    unsigned block_index = offset % BLOCK_CAPACITY;
    if (vn->fs->file->data[block_num]->block_size <= block_index) return -2;

    *byte = vn->fs->file->data[block_num]->block[block_index];
    return 0;
}

int 
file_stat (struct vnode *vn) {
    return vn->fs->file->size; 
}

int 
file_remove (struct vnode *dir, const char *file_name) {
    unsigned index;
    struct vnode *file = dir_lookup(dir, file_name, &index);
    char fbuf[256];
    if (file == NULL) { 
        sprintf(fbuf, "no such file: %s\r\n", file_name);
        kprintf(STDOUT, fbuf);
        return -1; /* no such file */ 
    }

    if (file->ref_count != 0) { 
        kprintf(STDOUT, "file is opened by another fd\r\n");
        return -1; /* file is opened by another fd */
    }
    release_file_data(file->fs->file);

    vnode_destroy(file);
    dir->fs->dir->files[index] = NULL;
    dir->fs->dir->file_count--;
    return 0;
}

