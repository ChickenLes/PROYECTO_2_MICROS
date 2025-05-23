/*
 * LCD.h
 *
 * Created: 4/28/2025 11:31:02 AM
 *  Author: itzle
 */ 


#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdlib.h>  
#include <util/delay.h>

void LCD_CLEAR(); 
void LCD_CONF();                   //MODULO DE CONFIGURACION
void LCD_WRITE(char *texto);      //MODULO PARA ESCRIBIR TEXTO
void LCD_CURSOR(uint8_t fila);    //ESCRIBIR EN LA LINEA 1 Y 2 DEL LCD

#endif /* LCD_H_ */