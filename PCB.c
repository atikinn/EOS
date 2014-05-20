/*
 * PCB.c
 *
 *  Created on: May 17, 2014
 *      Author: isinyagin
 */

#include <stdint.h>
#include <derivative.h>
#include "fd.h"
#include "alloc.h"
#include "process.h"
#include "queue.h"

#define STACK_SIZE 1024
#define MAX_PID 255

enum state {
	READY,
	RUNNING,
	SLEEP,
	ZOMBIE,
};

struct pcb {
	uint8_t pid;
	enum state state;
	void *stack;
    struct file_desc *(*fd_table)[OPEN_MAX];
    void *start;
};

struct pcb *curproc;

static uint8_t pid_map[MAX_PID];
static QUEUE *pcbq;

int get_pid() {
	__asm("cpsid i");
	for (int i = 1; i < MAX_PID; i++) {
		if (pid_map[i] == 0) {
			__asm("cpsie i");
			return i;
		}
	}
	__asm("cpsie i");
	return -1;
}

int free_pid(uint8_t pid) {
	__asm("cpsid i");
	if (pid_map[pid] == 0) {
		__asm("cpsie i");
		return -1;
	}
	pid_map[pid] = 0;
	__asm("cpsie i");
	return 0;
}

struct pcb *pcb_create(uint8_t pid) {
	struct pcb *pcb = kmalloc(sizeof *pcb);
	if (!pcb) goto out;

	pcb->stack = kmalloc(STACK_SIZE);
	if (!pcb) goto out2;

	pcb->fd_table = kmalloc(sizeof pcb->fd_table);
	if (!pcb) goto out3;

	__asm("cpsid i");
	pcb->pid = pid;
	__asm("cpsie i");

	pcb->state = READY;
	return pcb;

out3:
	kfree(pcb->stack);
out2:
	kfree(pcb);
out:
	return NULL;
}

int pcb_destroy(struct pcb *pcb) {
	kfree(pcb->fd_table);
	kfree(pcb->stack);
	kfree(pcb);
	return 0;
}

int pcb_set_state(struct pcb *pcb, enum state state) {
	/* this function should always change the state */
	if (pcb->state == state) return -1;
    pcb->state = state;
	return 0;
}

void pcbq_init() {
	pcbq = kmalloc(sizeof *pcbq);
	if (pcbq == NULL) panic();
}

/* adds to the end of the list */
int pcb_enqueue(struct pcb *pcb) {
	QNODE *newnode = kmalloc (sizeof *newnode);
	if (newnode == NULL) return -1;
	newnode->pcb = pcb;
	queue_add(pcbq, newnode);
	return 0;
}

/* removes from the head of the list the first READY pcb */
struct pcb *pcb_dequeue() {
	/* pid 0 is always sleeping in the queue */
	__asm("cpsid i");
	for (int i = 0; i < pcbq->count; i++) {
		QNODE *node = queue_remove(pcbq);
		switch (node->pcb->state) {
		case READY: 
			queue_add(pcbq, node);
			__asm("cpsie i");
			return node->pcb;
			break;
		case RUNNING: 
		case SLEEP: 
			queue_add(pcbq, node);
			break;
		case ZOMBIE: 
			pcb_destroy(node->pcb);
			kfree(node);
			break;
		}
	}
    __asm("cpsie i");
    return NULL;
}

int kfork(int (*fptr) (int argc, char *argv[])) {
	int pid = get_pid();
	if (pid == -1) return -1;
	
	struct pcb *thread = pcb_create(pid);
	if (thread == NULL) goto out;

	pcb_enqueue(thread);

	return pid;

out:
	free_pid(pid);
	return -1;
}

int yield() {
	return 0;
}

int wait() {
	
	return 0;
}

int kill() {
	
	return 0;
}

int block() {
	
	return 0;
}

int eexit(int exitcode) {
	//pcb_dequeue();
	return 0;
}

void SysTick_Handler() {
	  uint32_t copyOfSP = 0;

	  //entry code to handler
	  //0:	b590      	push	{r4, r7, lr}
	  //2:	b083      	sub	sp, #12
	  //4:	af00      	add	r7, sp, #0

	  /* The following assembly language will push registers r4 through
	   * r11 onto the stack */
	  __asm("push {r4,r5,r6,r7,r8,r9,r10,r11}");

	  /* As stated in section B5.1 on page B5-798 of the ARM®v7-M
	   * Architecture Reference Manual Errata markup ARM DDI
	   * 0403Derrata 2010_Q3, "To support reading and writing the
	   * special-purpose registers under software control, ARMv7-M
	   * provides three system instructions, CPS, MRS, and MSR. */

	  /* The MRS (Move to Register from Special Register) instruction is
	   * described in section B5.2.2 on page B5-803 and the MSR (Move to
	   * Special Register from ARM Register) instruction is described in
	   * section B5.2.3 on page B5-805 */

	  /* One web site where the syntax for using advanced features in the
	   * 'asm' in-line assembler pseudo-function call is described is:
	   * http://www.ethernut.de/en/documents/arm-inline-asm.html */

	  /* The LDR (Load Register) instruction is described in section
	   * A7.7.44 on page A7-291 and the STR (Store Register) instruction
	   * is described in section A7.7.159 on page A7-475 */

	  /* The following assembly language will put the current main SP
	   * value into the local, automatic variable 'copyOfSP' */
	  __asm("mrs %[mspDest],msp" : [mspDest]"=r"(copyOfSP));

	  
	  __asm("ldr  r0, [%[shcsr]]"   "\n"
	           "and  r0, r0, %[mask]"  "\n"
	           "push {r0}"
	           :
	           : [shcsr] "r" (&SCB_SHCSR), [mask] "I" (SCB_SHCSR_SVCALLACT_MASK)
	           : "r0", "memory", "sp");
		
	  curproc = pcb_dequeue();
	  curproc->state = RUNNING;
	  uint32_t *stack = (uint32_t *)((char *)curproc->stack + STACK_SIZE);

	  /* The following assembly language will write the value of the
	   * local, automatic variable 'copyOfSP' into the main SP */
	  __asm("msr msp,%[mspSource]" : : [mspSource]"r"(stack) : "sp");

	  
	  __asm("pop {r0}"              "\n"
	           "ldr r1, [%[shcsr]]"    "\n"
	           "bic r1, r1, %[mask]"  "\n"
	           "orr r0, r0, r1"        "\n"
	           "str r0, [%[shcsr]]"
	           :
	           : [shcsr] "r" (&SCB_SHCSR), [mask] "I" (SCB_SHCSR_SVCALLACT_MASK)
	           : "r0", "r1", "sp", "memory");

	  /* The POP (Pop Multiple Registers) instruction is described in
	   * section A7.7.98 on page A7-387 of the ARM®v7-M Architecture
	   * Reference Manual Errata markup ARM DDI 0403Derrata 2010_Q3 */
	  *(--stack) = 0;
	  *(--stack) = 0;
	  *(--stack) = 0;
	  //*(--stack) = (uint32_t)(stack + 9);	TODO: doesn't compile
	  *(--stack) = 0;	/* R8 */
	  *(--stack) = 0;
	  *(--stack) = 0;
	  *(--stack) = 0;
	  
	  // local vars
	  *(--stack) = 0;
	  
	  *(--stack) = 0;	/* R4 */
	  *(--stack) = 0;	/* R7 */
	  *(--stack) = 0xfffffff9;	/* LR */
	  
	  *(--stack) = 0;
	  *(--stack) = 0;

	  *(--stack) = 0;
	  *(--stack) = 0;
	  *(--stack) = 0;
	  *(--stack) = (uint32_t)(&kill);
	  *(--stack) = (uint32_t)(&curproc->start);
	  *(--stack) = 0x1000000;

	  /* The following assembly language will pop registers r4 through
	   * r11 off of the stack */
	  __asm("pop {r4,r5,r6,r7,r8,r9,r10,r11}");
}

void init_systick() {
	SYST_RVR = 10000;
	SYST_CVR = 0; // Reset the Current Value
	SYST_CSR = SysTick_CSR_ENABLE_MASK | SysTick_CSR_TICKINT_MASK | SysTick_CSR_CLKSOURCE_MASK;
}
