/*
 * EEPROM_LIB.h
 *
 * Created: 5/10/2025 11:58:10 AM
 *  Author: itzle
 */

#define F_CPU 16000000LU
#include "EEPROM_LIB.h"
#include <util/delay.h>


void EEPROM_write(uint16_t address, uint8_t data) {
	while (EECR & (1 << EEPE)); // Esperar si hay escritura en curso
	
	EEAR = address;             // Establecer dirección
	EEDR = data;                // Establecer dato
	
	EECR |= (1 << EEMPE);       // Habilitar escritura
	EECR |= (1 << EEPE);        // Iniciar escritura
}


uint8_t EEPROM_read(uint16_t address) {

	while(EECR & (1 << EEPE));
	
	//VER LA DIRECCION
	EEAR = address;
	
	//LEER DIRECCION
	EECR |= (1 << EERE);
	
	return EEDR;
}

void escribir_eeprom(uint16_t addr, uint16_t *datos, uint8_t n) {
	for(uint8_t i = 0; i < n; i++) {
		uint8_t low = datos[i] & 0xFF;
		uint8_t high = (datos[i] >> 8) & 0xFF;
		
		// Escribir con verificación
		do {
			EEPROM_write(addr + i*2, low);
			_delay_ms(10);
		} while(EEPROM_read(addr + i*2) != low);
		
		do {
			EEPROM_write(addr + i*2 + 1, high);
			_delay_ms(10);
		} while(EEPROM_read(addr + i*2 + 1) != high);
	}
}

void leer_eeprom(uint16_t addr, uint16_t *datos, uint8_t n) {
	for(uint8_t i = 0; i < n; i++) {
		uint8_t low = EEPROM_read(addr + i*2);
		uint8_t high = EEPROM_read(addr + i*2 + 1);
		datos[i] = (high << 8) | low;
	}
}