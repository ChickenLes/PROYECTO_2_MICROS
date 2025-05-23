/*
 * PIN_CHANGE.h
 *
 * Created: 8/05/2025 14:31:39
 *  Author: Bren
 */ 


#ifndef PIN_CHANGE_H_
#define PIN_CHANGE_H_

#include <avr/io.h>
#include <avr/interrupt.h>


extern volatile uint8_t MODO_LCD; //MODO = 0, EEPROM. MODO = 1, MANUAL. MODO = 2, GUARDANDO
extern volatile uint16_t VALOR_JOYSTICK; //VALOR ACTUAL DEL JOYSTICK PARA CAMBIAR ENTRE SLOTS
extern volatile uint8_t PUSH_FLAG;

void PIN_CHANGE_CONF(void);
void ANTI_REBOTE(void);



#endif /* PIN_CHANGE_H_ */