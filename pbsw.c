#include "fs.h"
#include "dev.h"

void sw_init (struct vnode *vn) {
    SIM_SCGC5 |= (SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);
    PORT_PCR_REG(vn->dev->driver->port_base_ptr, vn->dev->driver->mask) = 
         PORT_PCR_MUX(PORT_PCR_MUX_GPIO) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* read */
static
int 
sw_status(struct vnode *vn, char *byte) {
    vn->dev->status = 
        !((vn->dev->driver->gpio_base_ptr->PDIR) & (1 << vn->dev->driver->mask));
    *byte = vn->dev->status;
    return 0;
}

int 
sw_read(struct vnode *vn, char *byte, int offset) {
    (void)offset;
    return sw_status(vn, byte);
}

/* change some state */
static
int 
sw_switch(struct vnode *vn, char byte) {
    vn->dev->status = byte;
    return 0;
}

int 
sw_write(struct vnode *vn, char byte) {
    return sw_switch(vn, byte);
}

const struct vfs_ops sw_ops = {
    .open  = dev_open,
    .close = dev_close,
    .write = sw_write,
    .read  = sw_read,
    .init  = sw_init
};
