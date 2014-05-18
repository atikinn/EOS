#ifndef _LED_H_
#define _LED_H_

#include "fs.h"

void led_init (struct vnode *vn);
int led_write(struct vnode *vn, char byte);
int led_read (struct vnode *vn, char *byte, int offset);

#endif  /* _LED_H_ */
