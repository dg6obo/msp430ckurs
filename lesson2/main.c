#include <msp430.h> 

/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;			// stop watchdog timer
	DCOCTL = 0; 						// sane (lowest) default
	BCSCTL1 = CALBC1_1MHZ; 				// set dco 1mhz
	DCOCTL = CALDCO_1MHZ;				// set dco 1mhz

	unsigned int i = 0;					// variable for delay

	P1DIR |= 0x01;						// configure gpio pin P1.0 (red)

	while (1) {
		__delay_cycles(20000);
		P1OUT ^= 0x01;
	}

	return 0;
}

