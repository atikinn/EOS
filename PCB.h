
#ifndef _PCB_H_
#define _PCB_H_

struct pcb;

int kfork(int (*fptr) (int argc, char *argv[]));
void pcbq_init();

#endif
