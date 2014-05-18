#ifndef _FS_H_
#define _FS_H_

enum fs_const {
    DIR_MAX = 10,
    FILE_MAX = 20,
    FILE_CAPACITY = 15,
    NONSEEK = -1,
    BLOCK_CAPACITY = 100,
};

enum fs_type { 
    DEV, 
    FS,
};

enum fs_entry_type { 
    DIRECTORY, 
    REG 
};

struct vnode {
    enum fs_type type;
    union {
        struct dev *dev;
        struct fs_entry *fs;
    };
    unsigned ref_count;
    const struct vfs_ops *ops;
};

/* symlinks can be added here */
struct fs_entry {
    int e_type;
    union {
        struct file *file;
        struct dir *dir;
    };
};

struct vfs_ops {
    int (*open) (struct vnode **, const char *);
    int (*close) (struct vnode *);
    int (*read) (struct vnode *, char *, int);
    int (*write) (struct vnode *, char);
    int (*create) (struct vnode *, enum fs_entry_type, char *);
    int (*remove) (struct vnode *, const char *);
    int (*stat) (struct vnode *);
    int (*mkdir) (struct vnode *);
    int (*rmdir) (struct vnode *);
    int (*readdir) (struct vnode *);
    void (*init) (struct vnode *);
};

struct vnode *vnode_create(enum fs_type type);
int vnode_remove(struct vnode *vn);
struct vnode *dir_lookup(struct vnode *cur, 
                         const char *file_name, 
                         unsigned *file_index);
struct fs_entry * fs_entry_create (enum fs_entry_type type);
int vnode_destroy(struct vnode *vn);
int is_dev(const char *path);
struct vnode *dir_sub_lookup(struct vnode *cur, const char *dir_name);

struct vnode *cwd;

#endif
