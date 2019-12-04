#ifndef __PULSE_H__
#define __PULSE_H__
#endif

unsigned char pulseCount = 0;	// Triggered by interrupt, reset every 5s

void PulseOn() {
	// Pin Change Mask 0 (PCMSK0) to A0
	PCMSK0 |= 0x01;
	
	// Pin Change Interrupt Control Register (PCICR) to A
	PCICR |= 0x01;
	
	// Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void PulseISR() {
	pulseCount++;
}

ISR(PCINT0_vect) {
	PulseISR();
}