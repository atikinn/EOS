#ifndef _H_FD_TABLE_H_
#define _H_FD_TABLE_H_
    
#define OPEN_MAX 128
struct file_desc *fd_table[OPEN_MAX];

enum con_id { /* for fd_table */
    STDIN, 
    STDOUT, 
    STDERR,
    STDNONE,
    LCDOUT = 3,
};

struct file_desc {
    struct vnode *vn;
    unsigned offset;
};

struct file_desc * fd_create(struct vnode *vn, int offset);
int get_fd(void);
int fd_destroy(struct file_desc *fd);

#endif
