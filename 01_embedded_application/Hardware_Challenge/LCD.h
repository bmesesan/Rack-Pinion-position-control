/*
 * LCD.h
 *
 * Created: 3/24/2020 2:12:23 PM
 *  Author: Bogdan
 */ 


#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

//Sends the Enable signal to the LCD
void LCD_EN(void);

//Sends a 4-bit instruction to the LCD
void LCD_SendInstr(unsigned char val);

//Sends an 8-bit data value, which will be displayed on the LCD
void LCD_WriteData(unsigned char val);

//Sends all the necessary setup instructions to the LCD
void LCD_Setup(void);

//Writes a signed integer number on the LCD
void LCD_WriteInt(int num);

//Writes a string on the LCD
void LCD_WriteString(char *str);

//Writes a rational number, with 4 digit accuracy on the LCD
void LCD_WriteDouble(double num);

//Sets the position of the next character written to the LCD
void LCD_SetPosition(unsigned char pos);

//returns the mirrored number
unsigned int mirr_number(unsigned int num);

//returns the number of digits in a number
unsigned int nr_cif(unsigned int num);

//Writes the initial display
//A = Acceleration
//V = Velocity or Speed
//Pos = the current position of the pinion on the rack
//Ref = the desired position
void LCD_InitialDisplay(void);
void LCD_DisplayAcc(double acc);
void LCD_DisplayVel(double vel);
void LCD_DisplayPos(int pos);
void LCD_DisplayRef(int ref);

#endif /* LCD_H_ */