#ifndef _DIR_H_
#define _DIR_H_
    
struct dir {
    char *name;
    struct vnode *parent;
    struct vnode *subdir[DIR_MAX];
    struct vnode *files[FILE_MAX];
    unsigned file_count;
    unsigned subdir_count;
    const struct vfs_ops *ops;
};

int dir_create(struct vnode *vn, enum fs_entry_type type, char *path);
int dir_open (struct vnode **dir, const char *name);
int dir_close (struct vnode *dir);
int dir_write (struct vnode *vn, char byte);
int dir_read (struct vnode *vn, char *byte, int offset);
int dir_remove(struct vnode *dir, const char *name);
int dir_list (struct vnode *vn);

#endif
