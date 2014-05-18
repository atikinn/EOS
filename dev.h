#ifndef _DEV_H_
#define _DEV_H_

#include "derivative.h"

#ifndef PORT_PCR_MUX_ANALOG
#define PORT_PCR_MUX_ANALOG 0
#endif
#ifndef PORT_PCR_MUX_GPIO
#define PORT_PCR_MUX_GPIO 1
#endif

enum dev_family { 
    LED, 
    SW, 
    UART,
    LCD,
    ADC,
    HW
};

enum dev_mask { 
    LED_BLUE_PORTA_BIT   = 10,
    LED_ORANGE_PORTA_BIT = 11,
    LED_YELLOW_PORTA_BIT = 28,
    LED_GREEN_PORTA_BIT  = 29,

    PBTN_SW1_PORTD_BIT = 0,
    PBTN_SW2_PORTE_BIT = 26,

    NONE_BIT = 128
};

enum dev_id {   /* for array devices */
    ORANGE = 0,
    YELLOW,
    GREEN,
    BLUE,
    SW1,
    SW2,
    LCD1,
    HW1,
    HW2,
    HW3,
    HW4,
    PTN,
    THRM,
    UART2,
    DEV_MAX
};

struct driver {
    char *name;
    enum dev_id id;
    enum dev_mask mask; 
    union {	/* first ptr in the driver */
        GPIO_MemMapPtr gpio_base_ptr;
        UART_MemMapPtr uart_ptr;
        struct console *console;
        uint8_t addr;
    };
    PORT_MemMapPtr port_base_ptr;	/* second ptr in the driver */
};

struct dev {
    enum dev_family family; /* LED | SW | UART | LCD | ADC | HW */
    int status;
    const struct driver *driver;
};

struct vnode *devices[DEV_MAX];

extern const struct vfs_ops sw_ops;
extern const struct vfs_ops led_ops;
extern const struct vfs_ops uart_ops;
extern const struct vfs_ops lcd_ops;
extern const struct vfs_ops touch_ops;
extern const struct vfs_ops adc_ops;

void port_clock_init(void);
int dev_create(struct vnode *vn, enum dev_family family, enum dev_id id);
int dev_open(struct vnode **dev, const char *dev_name);
int dev_close(struct vnode *vn);

#endif
