/*
 * PIN_CHANGE_LIB.c
 *
 * Created: 5/3/2025 10:40:01 PM
 *  Author: itzle
 */ 

// --- PIN_CHANGE_LIB.c modificado ---
#define F_CPU 16000000UL
/* ------------ PIN_CHANGE_LIB.c ------------ */
#include "PIN_CHANGE_LIB.h"
#include <avr/interrupt.h>

volatile pcint_event_t evento_actual = PCINT_EVENT_NONE;
volatile uint8_t rebote_counter = 0;

ISR(PCINT1_vect) {
	static uint8_t estado_anterior = 0xFF;
	uint8_t estado_actual = PINC & ((1 << PC0) | (1 << PC1));
	
	if(estado_actual != estado_anterior) {
		_delay_us(500); // Pequeña espera para estabilizar
		
		if(!(estado_actual & (1 << PC0)) && (estado_anterior & (1 << PC0))) {
			evento_actual = PCINT_EVENT_PC0_PRESSED;
		}
		else if(!(estado_actual & (1 << PC1)) && (estado_anterior & (1 << PC1))) {
			evento_actual = PCINT_EVENT_PC1_PRESSED;
		}
		
		estado_anterior = estado_actual;
	}
}

pcint_event_t PCINT_GetEvent() {
	pcint_event_t tmp = evento_actual;
	evento_actual = PCINT_EVENT_NONE;
	return tmp;
}

// Llamar en el loop principal
void PCINT_DebounceUpdate() {
	if (debounce_counter > 0) {
		debounce_counter--;
		if (debounce_counter == 0) {
			if (pc0_pending && !(PINC & (1 << PINC0))) {
				last_event = PCINT_EVENT_PC0_PRESSED;
				current_mode = !current_mode;
			}
			if (pc1_pending && !(PINC & (1 << PINC1))) {
				last_event = PCINT_EVENT_PC1_PRESSED;
				current_value = (current_value % 4) + 1;
			}
			pc0_pending = 0;
			pc1_pending = 0;
		}
	}
}

void PCINT_Init() {
	DDRC &= ~((1 << PINC0) | (1 << PINC1));
	PORTC |= (1 << PINC0) | (1 << PINC1);
	PCMSK1 = (1 << PCINT8) | (1 << PCINT9);
	PCICR |= (1 << PCIE1);
}

pcint_event_t PCINT_GetEvent() {
	pcint_event_t tmp = last_event;
	last_event = PCINT_EVENT_NONE;
	return tmp;
}

uint8_t PCINT_GetCurrentMode() { return current_mode; }
uint8_t PCINT_GetCurrentValue() { return current_value; }
