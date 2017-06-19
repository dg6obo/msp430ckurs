#include <msp430.h> 

/*
 * a shot demo for using an lcd dot matrix module on a msp430 launchpad
 *
 * for simplicity only P1 is used - so even the smallest msp430 are suitable
 *
 * P1.0 and P1.1 are register select and enable pins on the module, data pins
 * are mapped to P1.4 to P1.7 - little more difficult wiring as pins are on
 * opposite sides of the mcu, but allows easier logic through shifting.
 *
 */

#define     MODULE_PIN_RS            BIT0          // P1.0
#define     MODULE_PIN_EN            BIT1          // P1.1
#define     MODULE_PIN_D7            BIT7          // P1.7
#define     MODULE_PIN_D6            BIT6          // P1.6
#define     MODULE_PIN_D5            BIT5          // P1.5
#define     MODULE_PIN_D4            BIT4          // P1.4

/*
 * predefined pin mask used for setting P1DIR and P1OUT
 */

#define     MODULE_PIN_MASK  ((MODULE_PIN_RS | MODULE_PIN_EN | MODULE_PIN_D7 | MODULE_PIN_D6 | MODULE_PIN_D5 | MODULE_PIN_D4))

/*
 * emulate boolean
 */

#define     FALSE                 0
#define     TRUE                  1
#define     PULSE_DELAY 		  200

/*
 * pulse EN signal - needed to tell the lcd module to read data from the data bus
 * delays are necessary as the module needs some processing time after reading
 */
void pulseMoudle() {
	// pull EN signal low - should already be low, but be sure
	P1OUT &= ~MODULE_PIN_EN;
	__delay_cycles(PULSE_DELAY);

	// pull EN signal high
	P1OUT |= MODULE_PIN_EN;
	__delay_cycles(PULSE_DELAY);

	// pull EN signal low again
	P1OUT &= (~MODULE_PIN_EN);
	__delay_cycles(PULSE_DELAY);
}

/*
 * write a single byte to lcd module in 4 bit mode, msb nibble first, lsb nibble second
 *
 * outByte - byte to send
 * sendAsChar - send as char data (TRUE) or command (FALSE)
 */

void SendByte(char outByte, int sendAsChar) {
	// clear out all pins
	P1OUT &= (~MODULE_PIN_MASK);

	// send msb (high nibble) via simple mask 
	P1OUT |= (outByte & 0xF0);

	if (sendAsChar == TRUE) {
		P1OUT |= MODULE_PIN_RS;
	} else {
		P1OUT &= ~MODULE_PIN_RS;
	}

	// pulse EN line, so the module will read data from P1.4-P1.7
	pulseMoudle();

	// clear out all pins (not really necessary, but be safe)
	P1OUT &= (~MODULE_PIN_MASK);

	// send lsb (low nibble), via mask and shift
	P1OUT |= ((outByte & 0x0F) << 4);

	if (sendAsChar == TRUE) {
		P1OUT |= MODULE_PIN_RS;
	} else {
		P1OUT &= ~MODULE_PIN_RS;
	}

	// pulse EN line, so the module will read data from P1.4-P1.7
	pulseMoudle();
}

/*
 * postion the module cursor via module command
 * 
 * see docs for detailed address calculation, first row starts on 0x00, second on 0x40
 */

void LcmSetCursorPosition(char row, char column) {
	char address;

	if (row == 0) {
		address = 0;
	} else {
		address = 0x40;
	}

	address |= column;

	SendByte(0x80 | address, FALSE);
}

/*
 * clear screen via command
 */
void clearScreen() {
	//
	// Clear display, return home
	//
	SendByte(0x01, FALSE);
	SendByte(0x02, FALSE);
}

/*
 * initlialize lcd module with some useful defaults and in 4bit mode
 * 
 * hint: no not call more than once after the module was powered up
 */

void initMoudle(void) {
	// set port direction and set all outputs low
	P1DIR |= MODULE_PIN_MASK;
	P1OUT &= ~(MODULE_PIN_MASK);

	// wait for lcd module to boot, takes MUCH longer than any msp
	__delay_cycles(100000);

	// init to 4 bit mode, step one
	P1OUT &= ~MODULE_PIN_RS;
	P1OUT &= ~MODULE_PIN_EN;

	P1OUT = 0x20;
	PulseLcm();

	// init to 4 bit mode, step two
	SendByte(0x28, FALSE);

	// turn display on with cursor
	SendByte(0x0E, FALSE);

	// cursor auto increment
	SendByte(0x06, FALSE);
}

/*
 * "print" a string, send one byte at a time
 */
void printString(char *text) {
	char *c;
	c = Text;

	while ((c != 0) && (*c != 0)) {
		SendByte(*c, TRUE);
		c++;
	}
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	InitializeLcm();
	ClearLcmScreen();
	printString("01234567");
	while (1) {
		__delay_cycles(1000);
	}

}
