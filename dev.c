#include <string.h>
#include <stdlib.h>
#include "dev.h"
#include "fs.h"
#include "utils.h" 
#include "led.h"
#include "pbsw.h"
#include "uart.h"
#include "alloc.h"

#define ADC_CHANNEL_POTENTIOMETER   	0x14
#define ADC_CHANNEL_TEMPERATURE_SENSOR  0x1A

extern struct console console;

/* that is a very poor design choice for the drivers, but it's too late to reimplement it */
/* I should've used LEDs and pushbuttons the same manner I use other devices */
const struct driver drivers[] = { 
    { "led_or", ORANGE , LED_ORANGE_PORTA_BIT, .gpio_base_ptr = PTA_BASE_PTR, PORTA_BASE_PTR },
    { "led_yl", YELLOW , LED_YELLOW_PORTA_BIT, .gpio_base_ptr = PTA_BASE_PTR, PORTA_BASE_PTR },
    { "led_gr", GREEN  , LED_GREEN_PORTA_BIT , .gpio_base_ptr = PTA_BASE_PTR, PORTA_BASE_PTR },
    { "led_bl", BLUE   , LED_BLUE_PORTA_BIT  , .gpio_base_ptr = PTA_BASE_PTR, PORTA_BASE_PTR },

    { "sw1"   , SW1    , PBTN_SW1_PORTD_BIT  , .gpio_base_ptr = PTD_BASE_PTR, PORTD_BASE_PTR },
    { "sw2"   , SW2    , PBTN_SW2_PORTE_BIT  , .gpio_base_ptr = PTE_BASE_PTR, PORTE_BASE_PTR },
    
    {"lcd"    , LCD1      , 0					 , .console = &console , 	  NULL },

    {"hw1"    , HW1      , 0					 , { NULL }		 , 	  NULL },
    {"hw2"    , HW2      , 0					 , { NULL }		 , 	  NULL },
    {"hw3"    , HW3      , 0					 , { NULL }		 , 	  NULL },
    {"hw4"    , HW4      , 0					 , { NULL }		 , 	  NULL },
    
    {"ptn"    , PTN      , 0 , .addr = ADC_CHANNEL_POTENTIOMETER ,  	  NULL },
    {"tmr"    , THRM     , 0 , .addr = ADC_CHANNEL_TEMPERATURE_SENSOR  , 	  NULL },

    //{"ser"    , UART2   , 0					 , .uart_ptr = UART2_BASE_PTR, 	  NULL },

    { NULL    , DEV_MAX,  NONE_BIT           , .gpio_base_ptr = NULL 	 , 	  NULL }
};

int
dev_create(struct vnode *vn, enum dev_family family, enum dev_id id) {
    vn->dev = kmalloc (sizeof *vn->dev);
    if (vn->dev == NULL) return -1;
    vn->dev->family = family;
    vn->dev->driver = &drivers[id];
    switch(family) {
        case LED:
            vn->ops = &led_ops;
            break;
        case SW:
            vn->ops = &sw_ops;
            break;
        case UART: 
        	vn->ops = &uart_ops;
        	break;
        case LCD:
        	vn->ops = &lcd_ops;
        	break;
        case ADC:
        	vn->ops = &adc_ops;
        	break;
        case HW:
        	vn->ops = &touch_ops;
        	break;
        default: 
        	panic();
        	//return -1;
    }
    return 0;
}

int 
dev_close(struct vnode *vn) {
	vn->dev->status = 0;
    vn->ref_count--;
    return 0; 
}

int 
dev_open(struct vnode **dev, const char *dev_name) {
    enum dev_id id;
    for (id = 0; id < UART2; id++)
        if (!strcmp(dev_name, devices[id]->dev->driver->name)) break;
    if (id == UART2) return -1;
    //if (id == DEV_MAX) return -1;
    devices[id]->dev->status = 1;
    *dev = devices[id];
    return 0;
}
