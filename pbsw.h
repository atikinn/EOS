#ifndef _PB_H_
#define _PB_H_

int sw_write (struct vnode *vn, char byte);
int sw_read (struct vnode *vn, char *byte, int offset);
void sw_init (struct vnode *vn);

#endif
