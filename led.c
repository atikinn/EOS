#include "fs.h"
#include "dev.h"

void
led_init(struct vnode *vn) {
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; 
	PORT_PCR_REG(vn->dev->driver->port_base_ptr, vn->dev->driver->mask) = 
            PORT_PCR_MUX(PORT_PCR_MUX_GPIO);
	vn->dev->driver->gpio_base_ptr->PDDR |= 1<<vn->dev->driver->mask;
}

/* read */
static
int 
led_status(struct vnode *vn, char *byte) {
    *byte = vn->dev->status;
    return 0;
}

int
led_read (struct vnode *vn, char *byte, int offset) {
    (void)offset;
    return led_status(vn, byte);
}

/* write */
static
int 
led_switch(struct vnode *vn, char byte) {
	if (byte == 0) {
        vn->dev->driver->gpio_base_ptr->PSOR = 1<<(vn->dev->driver->mask);
        vn->dev->status = 0;
    } else {
        vn->dev->driver->gpio_base_ptr->PCOR = 1<<(vn->dev->driver->mask); 
        vn->dev->status = 1;
    }
    return 0;
}

int
led_write (struct vnode *vn, char byte) {
    return led_switch(vn, byte);
}

const struct vfs_ops led_ops = {
    .open  = dev_open,
    .close = dev_close,
    .write = led_write,
    .read  = led_read,
    .init  = led_init
};
