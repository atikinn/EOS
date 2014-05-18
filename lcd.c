/*
 * lcd.c
 *
 *  Created on: May 12, 2014
 *      Author: Administrator
 */
#include "io.h"
#include "lcdcConsole.h"
#include "lcdc.h"
#include "fd.h"
#include "fs.h"
#include "dev.h"

void
lcd_init(struct vnode *vn) {
	lcdcInit();
	lcdcConsoleInit(&console);
}

int
lcd_read (struct vnode *vn, char *byte, int offset) {
	kprintf(STDERR, "this device cannot be read from\r\n");
	return 1;
}

int
lcd_write (struct vnode *vn, char byte) {
	lcdcConsolePutc(&console, byte);
	return 0;
}

const struct vfs_ops lcd_ops = {
    .open  = dev_open,
    .close = dev_close,
    .write = lcd_write,
    .read  = lcd_read,
    .init  = lcd_init
};
