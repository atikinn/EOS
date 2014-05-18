/**
 * intSerialIO.c
 * Serial input and output interrupt test program
 * 
 * ARM-based K70F120M microcontroller board
 *   for educational purposes only
 * CSCI E-92 Spring 2014, Professor James L. Frankel, Harvard Extension School
 *
 * Written by James L. Frankel (frankel@seas.harvard.edu)
 */

/*
 * Important note:
 * 
 * The file Project_Settings -> Startup_Code -> kinetis_sysinit.c needs to be modified so
 * that a pointer to the interruptSerialPort2 function is in the vector table at vector 65 (0x0000_0104)
 * for UART2 status sources.
 *
 * The following declaration needs to inserted earlier in the file:
 *   extern void interruptSerialPort2(void);
 *
 * If using the GCC Toolchain, the vector table is named "InterruptVector", and the line:
 *   Default_Handler,		(comment delimiters removed) Vector 65: UART2, Status Sources
 * needs to be changed to:
 *   interruptSerialPort2,	(comment delimiters removed) Vector 65: UART2, Status Sources
 *
 * If using the Freescale Toolchain, the vector table is named "__vect_table", and the line:
 *   (tIsrFunc)UNASSIGNED_ISR,       (comment delimiters removed) 65 (0x00000104) (prior: -)
 * needs to be changed to:
 *   (tIsrFunc)interruptSerialPort2, (comment delimiters removed) 65 (0x00000104) (prior: -)
 */

#include "derivative.h"
#include "intSerialIO.h"
#include "uart.h"
#include "nvic.h"
#include "fs.h"
#include "dev.h"
#include "fd.h"

/* The buffer to store characters input from serial port 2 */
static char serialPort2InputBuffer[SERIAL_PORT_2_INPUT_BUFFER_SIZE];
static int serialPort2InputEnqueueOffset = 0;
static int serialPort2InputDequeueOffset = 0;
static int serialPort2InputCharCount = 0;

/* The buffer to store characters to be output to serial port 2 */
static char serialPort2OutputBuffer[SERIAL_PORT_2_OUTPUT_BUFFER_SIZE];
static int serialPort2OutputEnqueueOffset = 0;
static int serialPort2OutputDequeueOffset = 0;
static int serialPort2OutputCharCount = 0;

static volatile int interruptCount = 0;
static volatile int interruptTDRECount = 0;
static volatile int interruptRDRFCount = 0;
static volatile int interruptNeitherTDREnorRDRFCount = 0;

/*****************************************************************************/
/*                                                                           */
/*  Name: interruptSerialPort2                                               */
/*                                                                           */
/*  Parameters:                                                              */
/*    None                                                                   */
/*                                                                           */
/*  Return value:                                                            */
/*    None                                                                   */
/*                                                                           */
/*  Side effects:                                                            */
/*    The serialPort2InputBuffer and associated data structures may be       */
/*    updated.                                                               */
/*                                                                           */
/*****************************************************************************/
void interruptSerialPort2(void) {
	uint32_t status;
	char ch;
	
	interruptCount++;

	status = UART2_S1;
	
	if(!(status & UART_S1_TDRE_MASK) && !(status & UART_S1_RDRF_MASK)) {
		interruptNeitherTDREnorRDRFCount++;
	}
	
	if((UART2_C2 & UART_C2_TIE_MASK) && (status & UART_S1_TDRE_MASK)) {
		char ch;
		
		interruptTDRECount++;
		
		/* The Transmit Data Register Empty Flag indicates that the amount of data in
		 * the transmit buffer is less than or equal to the value indicated by
		 * TWFIFO[TXWATER] at some point in time since the flag has been cleared
		 * (See 57.3.5 on page 1911 of the K70 Sub-Family Reference Manual,
		 * Rev. 2, Dec 2011) */

		/* The UART FIFO Transmit Watermark register (UARTx_TWFIFO) is initialized so
		 * that TXWATER is 0 on reset (See 57.3.19 on page 1929 of the K70 Sub-Family
		 * Reference Manual, Rev. 2, Dec 2011) */
		
		/* To clear TDRE, read S1 when TDRE is set and then write to the UART data
		 * register (D) (See 57.3.5 on page 1911 of the K70 Sub-Family Reference Manual,
		 * Rev. 2, Dec 2011) */

		/* Disable the transmitter interrupt for UART2 using the UART2_C2 register
		 * (UART Control Register 2) (See 57.3.4 on page 1909 of the K70 Sub-Family
		 * Reference Manual, Rev. 2, Dec 2011) */
	    
		if(serialPort2OutputCharCount > 0) {
			/* There is a character in the output buffer to be transmitted */
			
			/* Dequeue the character */
			ch = serialPort2OutputBuffer[serialPort2OutputDequeueOffset++];
			serialPort2OutputDequeueOffset = serialPort2OutputDequeueOffset %
					                         SERIAL_PORT_2_OUTPUT_BUFFER_SIZE;
			serialPort2OutputCharCount--;

			/* write the character to the UART */
			UART_D_REG(UART2_BASE_PTR) = ch;
		}

		/* If there are no more characters in the output buffer, disable the transmitter
		 * interrupt */
		if(serialPort2OutputCharCount <= 0) {
			UART2_C2 &= ~UART_C2_TIE_MASK;
		}
	}

	if((UART2_C2 & UART_C2_RIE_MASK) && (status & UART_S1_RDRF_MASK)) {
		interruptRDRFCount++;

		/* The Receive Data Register Full Flag indicates that the number of datawords
		 * in the receive buffer is equal to or more than the number indicated by
		 * RWFIFO[RXWATER] (See 57.3.5 on page 1911 of the K70 Sub-Family Reference Manual,
		 * Rev. 2, Dec 2011) */

		/* The UART FIFO Receive Watermark register (UARTx_RWFIFO) is initialized so
		 * that RXWATER is 1 on reset (See 57.3.21 on page 1930 of the K70 Sub-Family
		 * Reference Manual, Rev. 2, Dec 2011) */
		
		/* To clear RDRF, read S1 when RDRF is set and then read the UART data register
		 * (D) (See 57.3.5 on page 1911 of the K70 Sub-Family Reference Manual, Rev. 2,
		 * Dec 2011) */

		/* read the character that caused the interrupt */
		ch = UART_D_REG(UART2_BASE_PTR);
	
		if(serialPort2InputCharCount < SERIAL_PORT_2_INPUT_BUFFER_SIZE) {
			/* There is room in the input buffer for another character */
			serialPort2InputBuffer[serialPort2InputEnqueueOffset++] = ch;
			serialPort2InputEnqueueOffset = serialPort2InputEnqueueOffset %
                                       	   SERIAL_PORT_2_INPUT_BUFFER_SIZE;
			serialPort2InputCharCount++;
		}

		/* If there is no room in the input buffer for this character; discard it */
	}
}

/*****************************************************************************/
/*                                                                           */
/*  Name: getcharFromBuffer                                                  */
/*                                                                           */
/*  Parameters:                                                              */
/*    None                                                                   */
/*                                                                           */
/*  Return value:                                                            */
/*    Type    Description                                                    */
/*    char    the next character input from serial port 2.  This character   */
/*            will be retrieved from the serialPort2InputBuffer in FIFO      */
/*            fashion.                                                       */
/*                                                                           */
/*  Side effects:                                                            */
/*    The serialPort2InputBuffer and associated data structures may be       */
/*    updated.                                                               */
/*    Interrupts will be disabled and re-enabled by this routine.            */
/*                                                                           */
/*****************************************************************************/
char getcharFromBuffer(void) {
    char ch;

	/* Guarantee the following operations are atomic */

    /* Disable interrupts (PRIMASK is set) */
	__asm("cpsid i");

	while(serialPort2InputCharCount <= 0) {
    	/* No chars in the buffer; let's wait for at least one char to arrive */

		/* Allows interrupts (PRIMASK is cleared) */
		__asm("cpsie i");

		/* This is when an interrupt could occur */
		
		/* Disable interrupts (PRIMASK is set) */
		__asm("cpsid i");
    }

    /* A character should be in the buffer; remove the oldest one. */
    ch = serialPort2InputBuffer[serialPort2InputDequeueOffset++];
    serialPort2InputDequeueOffset = serialPort2InputDequeueOffset %
                                    SERIAL_PORT_2_INPUT_BUFFER_SIZE;
    serialPort2InputCharCount--;

    /* Allows interrupts (PRIMASK is cleared) */
	__asm("cpsie i");

	return ch;
}

/*****************************************************************************/
/*                                                                           */
/*  Name: putcharIntoBuffer                                                  */
/*                                                                           */
/*  Parameters:                                                              */
/*    Type    Description                                                    */
/*    char    the character to be output over serial port 2.  This character */
/*            will be buffered in the serialPort2OutputBuffer in FIFO        */
/*            fashion.                                                       */
/*                                                                           */
/*  Return value:                                                            */
/*    None                                                                   */
/*                                                                           */
/*  Side effects:                                                            */
/*    The serialPort2OutputBuffer and associated data structures may be      */
/*    updated.                                                               */
/*    Interrupts will be disabled and re-enabled by this routine.            */
/*                                                                           */
/*****************************************************************************/
void putcharIntoBuffer(char ch) {
	/* Guarantee the following operations are atomic */

    /* Disable interrupts (PRIMASK is set) */
	__asm("cpsid i");

	while(serialPort2OutputCharCount >= SERIAL_PORT_2_OUTPUT_BUFFER_SIZE) {
    	/* The buffer is full; let's wait for at least one char to be removed */

	    /* Allows interrupts (PRIMASK is cleared) */
		__asm("cpsie i");

		/* This is when an interrupt could occur */
		
		/* Disable interrupts (PRIMASK is set) */
		__asm("cpsid i");
	}

	/* There is room in the output buffer for another character */
	serialPort2OutputBuffer[serialPort2OutputEnqueueOffset++] = ch;
	serialPort2OutputEnqueueOffset = serialPort2OutputEnqueueOffset %
									 SERIAL_PORT_2_OUTPUT_BUFFER_SIZE;
	serialPort2OutputCharCount++;

	/* Enable the transmitter interrupt for UART2 using the UART2_C2 register
	 * (UART Control Register 2) (See 57.3.4 on page 1909 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
	UART2_C2 |= UART_C2_TIE_MASK;

	/* Allows interrupts (PRIMASK is cleared) */
	__asm("cpsie i");
}

/*****************************************************************************/
/*                                                                           */
/*  Name: putsIntoBuffer                                                     */
/*                                                                           */
/*  Parameters:                                                              */
/*    Type    Description                                                    */
/*    s       pointer to the string to be output over serial port 2.         */
/*            This string will be buffered in the serialPort2OutputBuffer    */
/*            in FIFO fashion.                                               */
/*                                                                           */
/*  Return value:                                                            */
/*    None                                                                   */
/*                                                                           */
/*  Side effects:                                                            */
/*    The serialPort2OutputBuffer and associated data structures may be      */
/*    updated.                                                               */
/*    Interrupts will be disabled and re-enabled by this routine.            */
/*                                                                           */
/*****************************************************************************/
void putsIntoBuffer(char *s) {
	while(*s) {
		putcharIntoBuffer(*s++);
	}
}

/********************************************************************/
/*
 * Initialize the specified UART in 8-N-1 mode with interrupts disabled
 * and with no hardware flow-control
 *
 * Note: This routine *does* enable the appropriate UART and PORT clocks
 *
 * Parameters:
 *  uartChannel UART channel to initialize
 *  clockInKHz  UART module clock in KHz (used to set the baud rate
 *              generator; see note above about the different module
 *              clocks used for each UART)
 *  baud        desired UART baud rate
 */
static
void 
uartInit(UART_MemMapPtr uartChannel, int clockInKHz, int baud) {
    uint16_t sbr, brfa;
    uint8_t temp;
    
	/* Enable the clock to the selected UART */    
    if(uartChannel == UART0_BASE_PTR) {
    	/* Enable clock for PORTF */
    	SIM_SCGC5 |= SIM_SCGC5_PORTF_MASK;

    	/* Enable clock for UART0 */
		SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;

    	/* Pin G1/PTF18 (See 10.3.1 on page 275 of the K70 Sub-Family Reference
		 * Manual, Rev. 2, Dec 2011).  Select the UART0_TXD function on PTF18 using the
		 * Pin Control Register (PORTx_PCRn) */
    	PORTF_PCR18 = PORT_PCR_MUX(0x4); // UART is ALT4 function for this pin

    	/* Pin F1/PTF17 */
       	/* Enable the UART0_RXD function on PTF17 */
    	PORTF_PCR17 = PORT_PCR_MUX(0x4); // UART is ALT4 function for this pin
    } else if (uartChannel == UART1_BASE_PTR) {
    	/* Enable clock for PORTE */
    	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

    	/* Enable clock for UART1 */
		SIM_SCGC4 |= SIM_SCGC4_UART1_MASK;

    	/* Pin E2/PTE0 */
		/* Enable the UART1_TXD function on PTE0 */
		PORTE_PCR0 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin

    	/* Pin F2/PTE1 */
		/* Enable the UART1_RXD function on PTE1 */
		PORTE_PCR1 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin
    } else if (uartChannel == UART2_BASE_PTR) {
    	/* Enable clock for PORTE */
    	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

    	/* Enable clock for UART2 */
    	SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;

    	/* Pin J3/PTE16 */
    	/* Enable the UART2_TXD function on PTE16 */
    	PORTE_PCR16 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin

    	/* Pin K2/PTE17 */
    	/* Enable the UART2_RXD function on PTE17 */
    	PORTE_PCR17 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin
    } else if(uartChannel == UART3_BASE_PTR) {
    	/* Enable clock for PORTC */
    	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;

    	/* Enable clock for UART3 */
    	SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;
    	
    	/* Pin E9/PTC17 */
        /* Enable the UART3_TXD function on PTC17 */
    	PORTC_PCR17 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin

    	/* Pin F9/PTC16 */
    	/* Enable the UART3_RXD function on PTC16 */
    	PORTC_PCR16 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin
    } else if(uartChannel == UART4_BASE_PTR) {
    	/* Enable clock for PORTC */
    	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;

    	/* Enable clock for UART4 */
    	SIM_SCGC1 |= SIM_SCGC1_UART4_MASK;
    	
    	/* Pin P7/PTE24 */
        /* Enable the UART4_TXD function on PTE24 */
    	PORTE_PCR24 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin

    	/* Pin R7/PTE25 */
    	/* Enable the UART4_RXD function on PTE25 */
    	PORTE_PCR25 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin
    } else {
    	/* Enable clock for PORTE */
    	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

    	/* Enable clock for UART5 */
    	SIM_SCGC1 |= SIM_SCGC1_UART5_MASK;

    	/* Pin H4/PTE8 */
        /* Enable the UART5_TXD function on PTE8 */
    	PORTE_PCR8 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin

    	/* Pin J1/PTE9 */
    	/* Enable the UART5_RXD function on PTE9 */
    	PORTE_PCR9 = PORT_PCR_MUX(0x3); // UART is ALT3 function for this pin
    }
                                
    /* Make sure that the transmitter and receiver are disabled while we 
     * change settings.
     */
    UART_C2_REG(uartChannel) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );

    /* Configure the UART for 8-bit mode, no parity */
    UART_C1_REG(uartChannel) = 0;	/* We need all default settings, so entire register is cleared */
    
    /* Calculate baud settings */
    sbr = (uint16_t)((clockInKHz * 1000)/(baud * 16));
        
    /* Save off the current value of the UARTx_BDH except for the SBR field */
    temp = UART_BDH_REG(uartChannel) & ~(UART_BDH_SBR(0x1F));
    
    UART_BDH_REG(uartChannel) = temp |  UART_BDH_SBR(((sbr & 0x1F00) >> 8));
    UART_BDL_REG(uartChannel) = (uint8_t)(sbr & UART_BDL_SBR_MASK);
    
    /* Determine if a fractional divider is needed to get closer to the baud rate */
    brfa = ((((uint32_t)clockInKHz*32000)/(baud * 16)) - (sbr * 32));
    
    /* Save off the current value of the UARTx_C4 register except for the BRFA field */
    temp = UART_C4_REG(uartChannel) & ~(UART_C4_BRFA(0x1F));
    
    UART_C4_REG(uartChannel) = temp |  UART_C4_BRFA(brfa);    

    /* Enable receiver and transmitter */
	UART_C2_REG(uartChannel) |= (UART_C2_TE_MASK | UART_C2_RE_MASK );
}

void intSerialIOInit(void) {
	/* On reset (i.e., before calling mcgInit), the processor clocking
	 * starts in FEI (FLL Engaged Internal) mode.  In FEI mode and with
	 * default settings (DRST_DRS = 00, DMX32 = 0), the MCGFLLCLK, the
	 * MCGOUTCLK (MCG (Multipurpose Clock Generator) clock), and the Bus
	 * (peripheral) clock are all set to 640 * IRC.  IRC is the Internal
	 * Reference Clock which runs at 32 KHz. [See K70 Sub-Family
	 * Reference Manual, Rev. 2, Section 25.4.1.1, Table 25-22 on
	 * page 657 and MCG Control 4 Register (MCG_C4) Section 25.3.4 on
	 * page 641] */
	
	/* After calling mcgInit, MCGOUTCLK is set to 120 MHz and the Bus
	 * (peripheral) clock is set to 60 MHz.*/

	/* Table 5-2 on page 221 indicates that the clock used by UART0 and
	 * UART1 is the System clock (i.e., MCGOUTCLK) and that the clock
	 * used by UART2-5 is the Bus clock. */
	const int IRC = 32000;		/* Internal Reference Clock */
	const int FLL_Factor = 1875;
	const int moduleClock = IRC * FLL_Factor;
	const int KHzInHz = 1000;

	const int baud = 115200;

	uartInit(UART2_BASE_PTR, moduleClock/KHzInHz, baud);

	/* Enable the receiver full interrupt for UART2 using the UART2_C2 register
	 * (UART Control Register 2) (See 57.3.4 on page 1909 of the K70 Sub-Family Reference
	 * Manual, Rev. 2, Dec 2011) */
	UART2_C2 |= UART_C2_RIE_MASK;

	/* Enable interrupts from UART2 status sources and set its interrupt priority */
	NVICEnableIRQ(UART2_STATUS_IRQ_NUMBER, UART2_STATUS_INTERRUPT_PRIORITY);
}

void
uart_init(struct vnode *vn) {
	(void)vn;
	intSerialIOInit();
}

int
uart_read (struct vnode *vn, char *byte, int offset) {
    (void)offset;
    *byte = getcharFromBuffer();
    return 0;
}

int
uart_write (struct vnode *vn, char byte) {
	(void)vn;
    putcharIntoBuffer(byte);
    /* probably no nee for that since we're using only byte i/o */
	//while((serialPort2OutputCharCount > 0) || !(UART2_S1 & UART_S1_TC_MASK)) { }
    //if (fd_table[LCDOUT] && fd_table[LCDOUT]->vn->dev->status) 
    	//lcdcConsolePutc(fd_table[LCDOUT]->vn->dev->driver->console, byte);
    return 0;
}

const struct vfs_ops uart_ops = {
    .open  = dev_open,
    .close = dev_close,
    .write = uart_write,
    .read  = uart_read,
    .init  = uart_init
};

#if 0
int main(void) {
	char c;
	
	printf("UART2 Receive FIFO size field is %d\n",
			(UART2_PFIFO&UART_PFIFO_RXFIFOSIZE_MASK) >> UART_PFIFO_RXFIFOSIZE_SHIFT);
	printf("UART2 Transmit FIFO size field is %d\n",
			(UART2_PFIFO&UART_PFIFO_TXFIFOSIZE_MASK) >> UART_PFIFO_TXFIFOSIZE_SHIFT);
	
	
	do {
		c = getcharFromBuffer();
		putsIntoBuffer("Received character ");
		putcharIntoBuffer(c);
		putsIntoBuffer("\r\n");
	} while(c != 'x');

	putsIntoBuffer("InterruptSerialIO project completed\r\n");

	/* Wait for the last character in the output buffer to be transmitted.  If this
	 * isn't done, then the program will terminate before all of the characters in the
	 * output buffer have been transmitted. */
	while((serialPort2OutputCharCount > 0) || !(UART2_S1 & UART_S1_TC_MASK)) {
	}

	printf("InterruptSerialIO project completed\n");

	return 0;
}

#endif
