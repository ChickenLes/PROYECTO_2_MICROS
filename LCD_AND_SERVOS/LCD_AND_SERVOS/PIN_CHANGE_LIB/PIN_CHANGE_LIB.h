/*
 * PIN_CHANGE_LIB.h
 *
 * Created: 5/3/2025 10:39:00 PM
 *  Author: itzle
 */ 


#ifndef PIN_CHANGE_LIB_H_
#define PIN_CHANGE_LIB_H_

#include <avr/io.h>
// Tipos de eventos

typedef enum {
	PCINT_EVENT_NONE,
	PCINT_EVENT_PC0_PRESSED,
	PCINT_EVENT_PC1_PRESSED
} pcint_event_t;

// Prototipos
void PCINT_Init();
pcint_event_t PCINT_GetEvent();
uint8_t PCINT_GetCurrentValue();
uint8_t PCINT_GetCurrentMode();
void PCINT_DebounceUpdate(void);


#endif /* PIN_CHANGE_LIB_H_ */