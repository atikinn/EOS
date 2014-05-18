#ifndef _FILE_H_
#define _FILE_H_

struct file {
    char *name;
    unsigned size;
    unsigned nblocks;
    struct file_block *data[FILE_CAPACITY];
};

struct file_block {
    unsigned block_size;
    char block[BLOCK_CAPACITY];
};

int file_create (struct vnode *dir, enum fs_entry_type type, char *path);
int file_open(struct vnode **file, const char *file_name);
int file_close (struct vnode *file);
int file_write (struct vnode *file, char byte);
int file_read (struct vnode *file, char *byte, int offset);
int file_remove (struct vnode *file, const char *file_name);
int file_stat (struct vnode *vn);


#endif
