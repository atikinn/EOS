#include "derivative.h"
#include "fs.h"
#include "dev.h"
#include "fd.h"
#include "io.h"

#define ADC_CFG1_MODE_8_9_BIT       0x0
#define ADC_CFG1_MODE_12_13_BIT     0x1
#define ADC_CFG1_MODE_10_11_BIT     0x2
#define ADC_CFG1_MODE_16_BIT        0x3
#define ADC_SC3_AVGS_32_SAMPLES     0x3

void adc_init(struct vnode *vn) {
   SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;
   ADC1_CFG1 = ADC_CFG1_MODE(ADC_CFG1_MODE_12_13_BIT);
   ADC1_SC3 = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(ADC_SC3_AVGS_32_SAMPLES);
}

unsigned int adc_read_helper(uint8_t channel) {
   ADC1_SC1A = channel;
   while(!(ADC1_SC1A & ADC_SC1_COCO_MASK)) {
   }
   return ADC1_RA;
}

int
adc_read(struct vnode *vn, char *byte, int offset) {
    (void)offset;
    // mask and return 
    *byte = adc_read_helper(vn->dev->driver->addr);
    return 0;
}

int
adc_write (struct vnode *vn, char byte) {
	(void)vn;
	(void)byte;
	kprintf(STDOUT, "you can't write to this device\r\n");
	return 1;
}

const struct vfs_ops adc_ops = {
    .open  = dev_open,
    .close = dev_close,
    .write = adc_write,
    .read  = adc_read,
    .init  = adc_init
};
