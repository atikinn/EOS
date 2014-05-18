#include "utils.h"
#include "fd.h"
#include "command.h"
#include "delay.h"
#include "PDB.h"

int
ser2lcd(void) {
	int lcd_fd = eopen("dev:lcd");
	if (lcd_fd == -1) return -1;

	int c;
	while ((c = egetc(STDIN)) != CHAR_EOF) {
		if (c == '\r') eputs(lcd_fd, "\r\n");
		eputc(lcd_fd, c);
	}
	eclose(lcd_fd);

	return 0;
}

int
touch2led(void) {
	int touch_fd1 = eopen("dev:hw1");
	int touch_fd2 = eopen("dev:hw2");
	int touch_fd3 = eopen("dev:hw3");
	int touch_fd4 = eopen("dev:hw4");
	int led_fd1 = eopen("dev:led_or");
	int led_fd2 = eopen("dev:led_yl");
	int led_fd3 = eopen("dev:led_gr");
	int led_fd4 = eopen("dev:led_bl");
	
	char a, b, c, d;
	while(1) {
		a = egetc(touch_fd1);
		b = egetc(touch_fd2);
		c = egetc(touch_fd3);
		d = egetc(touch_fd4);
		
		eputc(led_fd1, a);
		eputc(led_fd2, b);
		eputc(led_fd3, c); 
		eputc(led_fd4, d);
		if (a && b && c && d) break;
	}

    eputc(led_fd1, 0);
    eputc(led_fd2, 0);
    eputc(led_fd3, 0); 
    eputc(led_fd4, 0);

	eclose(touch_fd1);
	eclose(touch_fd2);
	eclose(touch_fd3);
	eclose(touch_fd4);
	eclose(led_fd1);
	eclose(led_fd2);
	eclose(led_fd3);
	eclose(led_fd4);
	
	return 0;
}

int
ptn2ser(void) {
	int fd = eopen("dev:ptn");
	unsigned char c;
	char buf[4] = { 0 };
	while((c = egetc(fd)) != 0) {
        char2ascii(c, buf);
		eputs(STDOUT, buf);
		eputs(STDOUT, "\r\n");
		delay(0x7ffff);
	}
	return 0;
}

int
thrm2ser(void) {
	int fd = eopen("dev:thrm");
	unsigned char c;
	char buf[4] = { 0 };
	while((c = egetc(fd)) != 0) {
        char2ascii(c, buf);
		eputs(STDOUT, buf);
		eputs(STDOUT, "\r\n");
		delay(0x7ffff);
	}
	return 0;
}

int
pb2led(void) {
	int led_fd1 = eopen("dev:led_or");
	int led_fd2 = eopen("dev:led_yl");
	int sw1_fd = eopen("dev:sw1");
	int sw2_fd = eopen("dev:sw2");
	
	char a, b;
	while (1) {
		a = egetc(sw1_fd);
		delay(0x7ffff);
		b = egetc(sw2_fd);
		delay(0x7ffff);
		if (a && b) break;
		eputc(led_fd1, a);
		eputc(led_fd2, b);
	}
	
	eclose(led_fd1);
	eclose(led_fd2);
	eclose(sw1_fd);
	eclose(sw2_fd);
	return 0;
}
