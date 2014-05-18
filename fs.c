#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "utils.h"
#include "dir.h"
#include "file.h"
#include "private.h"
#include "alloc.h"

struct vnode * 
vnode_create(enum fs_type type) {
    struct vnode *vn = kmalloc (sizeof *vn);
    if (vn == NULL) return NULL;
    memset(vn, 0, sizeof *vn);  /* set the union to zeros */
    vn->type = type;
    return vn;
}

int
vnode_destroy(struct vnode *vn) {
    switch (vn->type) {
        case FS:
            kfree(vn->fs);  /* free fs_entry */
            break;
        case DEV:   /* curerntly impossible */
            kfree(vn->dev);
            break;
    }
    kfree(vn);              /* free vnode */
    return 0;
}

struct fs_entry *
fs_entry_create (enum fs_entry_type type) {
    struct fs_entry *entry = (struct fs_entry *)kmalloc (sizeof *entry);  /* allocate fs_entry */
    if (entry == NULL) return NULL;
    memset(entry, 0, sizeof *entry);
    entry->e_type = type;
    return entry;
}

struct vnode *
dir_lookup(struct vnode *cur, const char *file_name, unsigned *file_index) {
    struct vnode *file = NULL;
    for (int i = 0; i < FILE_MAX; i++)
        if(cur->fs->dir->files[i] != NULL 
            && !strcmp(file_name, cur->fs->dir->files[i]->fs->file->name))  {
            file = cur->fs->dir->files[i];
            *file_index = i;
            break;
        }
    return file;
}

struct vnode *
dir_sub_lookup(struct vnode *cur, const char *dir_name) {
    struct vnode *dir = NULL;
    for (int i = 0; i < DIR_MAX; i++)
        if(cur->fs->dir->subdir[i] != NULL 
            && !strcmp(dir_name, cur->fs->dir->subdir[i]->fs->dir->name))  {
            dir = cur->fs->dir->subdir[i];
            break;
        }
    return dir;
}

int
is_dev(const char *path) {
    char *ptr = strstr(path, "dev:");
    if (ptr == path) return 1;
    return 0;
}

