/*
 * EEPROM_LIB.h
 *
 * Created: 5/10/2025 11:56:49 AM
 *  Author: itzle
 */ 


#ifndef EEPROM_LIB_H_
#define EEPROM_LIB_H_

#include <avr/io.h>
#include <avr/eeprom.h>

//ESCRIBIR
void EEPROM_write(uint16_t address, uint8_t data);

//LEER
uint8_t EEPROM_read(uint16_t address);

//ARRAY 16 BITS - PARA ESCRITURA
void escribir_eeprom(uint16_t addr, uint16_t *datos, uint8_t n);

//ARRAY 16 BITS - PARA LECTURA

void leer_eeprom(uint16_t addr, uint16_t *datos, uint8_t n);



#endif /* EEPROM_LIB_H_ */