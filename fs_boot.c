#include <stdlib.h>
#include <string.h>
#include "fd.h"
#include "fs.h"
#include "dev.h"
#include "dir.h"
#include "private.h"
#include "utils.h"
#include "io.h"
#include "mcg.h"
#include "sdram.h"
#include "flextimer.h"
#include "PDB.h"

static
int
console_init(void) {
    for (enum con_id con = STDIN; con < STDNONE; con++) {
        struct vnode *con_vnode = vnode_create(DEV);
        if (!con_vnode) return -1;

        if (dev_create(con_vnode, UART, UART2) == -1) {
        	vnode_destroy(con_vnode);
        	return -1;
        }

        con_vnode->ops->init(con_vnode);

        fd_table[con] = fd_create(con_vnode, NONSEEK);
        if (!fd_table[con]) return -1;
    }
    
    return 0;
}

static
int
cwd_init(void) {
    return dir_create(NULL, FS, "/");
}

static
struct vnode *
device_init_helper(enum dev_family family, enum dev_id id) {
	struct vnode *vn = vnode_create(DEV);
	if (vn == NULL) return NULL;

	if (dev_create(vn, family, id) == -1) {
		vnode_destroy(vn);
		return NULL;
	}

	vn->ops->init(vn);	
	if (family == LED) vn->ops->write(vn, 0);
	// put the console open logic here
	return vn;
}

static
int
device_init(void) {
    for (enum dev_id id = ORANGE; id != SW1; id++)
    	devices[id] = device_init_helper(LED, id);

    for (enum dev_id id = SW1; id != LCD1; id++)
    	devices[id] = device_init_helper(SW, id);

    devices[LCD1] = device_init_helper(LCD, LCD1);

    for (enum dev_id id = HW1; id != PTN; id++)
    	devices[id] = device_init_helper(HW, id);

    devices[PTN] = device_init_helper(ADC, PTN);
    devices[THRM] = device_init_helper(LCD, THRM);
    // TODO: make UART also a legal device
    return 0;
}

/*
 * fd_table[0-2] to not NULL for console
 * creates root directory
 * creates all devices
 */
int boot(void) {
	mcg_init();
	sdram_init();
    flexTimer0Init(1875);
    flexTimer0Start();

    PDB0Init(46875, PDBTimerOneShot);

    int rv;
    if ((rv = console_init()) == -1) panic();
    if ((rv = device_init()) == -1) kprintf(STDOUT, "device_init failed\r\n"); 
    if ((rv = cwd_init()) == -1) kprintf(STDOUT, "cwd_init failed\r\n");
    return rv;
}
