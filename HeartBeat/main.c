/*
 * HeartBeat.c
 *
 * Author : carmi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "HeartMatrix.c"
#include "Timer.c"

#include "nokia5110.h"
#include "nokia5110.c"

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char inputA0 = 0x00;	// PORTA0 temporary char, pulse indicator
unsigned char outputC = 0xFF;	// PORTC0 temporary char, 10s indicator

unsigned short timeCount = 0;	// Increments every 50ms
unsigned short timeForPulseCount = 0;
unsigned short timeForPulse = 0;
unsigned char pulseCount = 0;	// Triggered by interrupt, reset every 5s

unsigned char bpm = 0;

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

enum PulseStates {init, chill, pulse} pulseState;
void showHeartBPM() {
	timeForPulse = 60000 / bpm;
	
	switch(pulseState) {
		case init:
			timeForPulseCount = 0;
			break;
		case chill:
			timeForPulseCount++;
			outputC = 0x18;
			break;
		case pulse:
			timeForPulseCount = 0;
			outputC = 0x66;
			break;
	}
	
	switch(pulseState) {
		case init:
			pulseState = chill;
			break;
		case chill:
			if(timeForPulseCount < (timeForPulse / 50)) {
				pulseState = chill;
			} else {
				pulseState = pulse;
			}
			break;
		case pulse:
			if(timeCount < 200) {
				pulseState = chill;
			} else {
				pulseState = init;
			}
			break;
	}
	
}

void calculateBPM() {
	timeCount++;
	
	if(timeCount >= 200) {	// 50ms * 200 == 10000ms == 10s
		bpm = pulseCount * (6 / 2); // Divided by 2 because the rising and falling edge both trigger the pin change, so compensate
		pulseCount = 0;
		timeCount = 0;
	}
}

void displayBPM(unsigned char newBPM) {
	unsigned char digit;
	
	nokia_lcd_clear();
	nokia_lcd_write_string("BPM:", 1);
	
	digit = newBPM % 10;
	newBPM = newBPM / 10;
	nokia_lcd_set_cursor(40, 15);
	nokia_lcd_write_char(digit + '0', 3);
	
	digit = newBPM % 10;
	newBPM = newBPM / 10;
	nokia_lcd_set_cursor(20, 15);
	nokia_lcd_write_char(digit + '0', 3);
	
	digit = newBPM % 10;
	newBPM = newBPM / 10;
	nokia_lcd_set_cursor(0, 15);
	nokia_lcd_write_char(digit + '0', 3);
	nokia_lcd_render();
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;	// PORTA as input
	DDRB = 0xFF; PORTB = 0x00;	// PORTB as output
	DDRC = 0xFF; PORTC = 0x00;	// PORTC as output
	DDRD = 0xFF; PORTD = 0x00;	// PORTD as output
	
	inputA0 = (PINA & 0x01);
	
	TimerSet(50);
	TimerOn();
	
	PulseOn();
	
	nokia_lcd_init();
	nokia_lcd_clear();
	
    pulseState = init;
	pulseCount = 0;
	timeCount = 0;
	
    while(1) {
		calculateBPM();
		displayBPM(bpm);
		showHeartBPM();
		
		inputA0 = PINA & 0x01;
		
		while(!TimerFlag);
		TimerFlag = 0;
		
		PORTC = outputC;
    }
}