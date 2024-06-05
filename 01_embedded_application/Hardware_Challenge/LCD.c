/*
 * LCD.c
 *
 * Created: 3/24/2020 2:13:04 PM
 *  Author: Bogdan
 */ 

 #include "LCD.h"

 //Sends the Enable signal to the LCD
 void LCD_EN(void)
 {
	 PORTD &= ~(1 << PORTD4);
	 PORTD |= (1 << PORTD4);
	 _delay_us(40);
	 PORTD &= ~(1 << PORTD4);
 }

//Sends a 4-bit instruction to the LCD
 void LCD_SendInstr(unsigned char val)
 {
	 //Set RS and R/W on 0
	 PORTC &= ~(1 << PORTC0 | 1 << PORTC1);
	 PORTC &= ~(0x0F << 2);
	 PORTC |= (val << 2);
	 LCD_EN();
 }

//Sends an 8-bit data value, which will be displayed on the LCD
 void LCD_WriteData(unsigned char val)
 {
	 unsigned char aux;

	 PORTC &= ~(1 << PORTC0 | 1 << PORTC1);
	 PORTC &= ~(0x0F << 2);

	 //Set RS to 1 and R/W on 0
	 PORTC |= (1 << PORTC0);

	 //send the H part = the most significant 4 bits
	 aux = (val >> 4);
	 PORTC &= ~(0x0F << 2);
	 PORTC |= (aux << 2);
	 LCD_EN();

	 //send the L part = the least significant 4 bits
	 aux = (val << 4);
	 aux = (aux >> 4);
	 PORTC &= ~(0x0F << 2);
	 PORTC |= (aux << 2);
	 LCD_EN();
 }

//Sends all the necessary setup instructions to the LCD
 void LCD_Setup(void)
 {
	 unsigned char instr[8] = {3, 3, 3, 2, 2, 1, 0, 12};
	 unsigned char i;

	 for (i = 0; i < 8; i++)
	 {
		 LCD_SendInstr(instr[i]);
		 _delay_ms(1);
	 }
 }

//Writes a signed integer number on the LCD
 void LCD_WriteInt(int num)
 {
	 unsigned int val;
	 unsigned int cif;

	 if (num < 0)
	 {
		 LCD_WriteData('-');
		 num = num * (-1);
	 }

	 //we send the digits in reverse order
	 val = mirr_number(num);
	 //in case the number ends with a 0 we need to know the number of digits it has
	 cif = nr_cif(num);

	 while (val > 0)
	 {
		 //write digit by digit
		 LCD_WriteData(val % 10 + 48);
		 val = val / 10;
		 cif--;
	 }
	 if (cif > 0 || num == 0)
	 {
		 //case in which the original number ends with a '0' digit, we send out one more '0'
		 LCD_WriteData('0');
		 if (cif > 0)
			cif--;
		 while (cif > 0)
		 {
			LCD_WriteData('0');
			cif--;
		 }
	 }
 }

 //returns the mirrored number of the input
 unsigned int mirr_number(unsigned int num)
 {
	 unsigned int val = 0;
	 while (num > 0)
	 {
		 val = val * 10 + num % 10;
		 num = num / 10;
	 }
	 return (val);
 }

 //returns the number of digits in a given number
 unsigned int nr_cif(unsigned int num)
 {
	 unsigned int sum = 0;

	 if (num == 0)
		return (1);

	 while (num > 0)
	 {
		 sum++;
		 num = num / 10;
	 }

	 return (sum);
 }

//Writes a string on the LCD
 void LCD_WriteString(char *str)
 {
	 unsigned int i = 0;

	 while (str[i] != 0)
	 {
		 LCD_WriteData(str[i]);
		 i++;
	 }
 }

//Writes a rational number, with 4 digit accuracy on the LCD
 void LCD_WriteDouble(double num)
 {
	 int aux;

	 if (num < 0)
	 {
		 LCD_WriteData('-');
		 num = num * (-1);
	 }
	 //write the integer part of the number
	 aux = (int)(num);
	 LCD_WriteInt(aux);

	 //find the rational part of the number and display it
	 num = (double)(num - aux);
	 num = num * 10000;
	 aux = (int)(num);
	 LCD_WriteData('.');
	 if (aux < 1000)
	 {
		LCD_WriteInt(0);
		if (aux < 100)
		{
			LCD_WriteInt(0);
			if (aux < 10)
			{
				LCD_WriteInt(0);
				LCD_WriteInt(aux);
			}
			else
			{
				LCD_WriteInt(aux);
			}
		}
		else
		{
			LCD_WriteInt(aux);
		}
	}
	else
	{
		LCD_WriteInt(aux);
	}
 }

//Sets the position of the next character written to the LCD
 void LCD_SetPosition(unsigned char pos)
 {
	 unsigned char aux;

	 //Send High value
	 aux = 0b00001000 + (pos >> 4);
	 LCD_SendInstr(aux);

	 //Send Low value
	 aux = (pos << 4);
	 aux = (aux >> 4);
	 LCD_SendInstr(aux);
 }

//Writes the initial display
//A = Acceleration
//V = Velocity or Speed
//Pos = the current position of the pinion on the rack
//Ref = the desired position
 void LCD_InitialDisplay(void)
 {
	LCD_SetPosition(0);
	LCD_WriteString("V:");

	LCD_SetPosition(12);
	LCD_WriteString("Pos:");

	LCD_SetPosition(20);
	LCD_WriteString("A:");

	LCD_SetPosition(32);
	LCD_WriteString("Ref:");
 }

  void LCD_DisplayAcc(double acc)
  {
	  unsigned int pos = nr_cif((int)(acc)) + 22 + 5;

	  LCD_SetPosition(22);
	  LCD_WriteDouble(acc);

	  if (acc < 0)
		pos++;
	  LCD_SetPosition(pos);
	  while (pos < 32)
	  {
		  LCD_WriteData(' ');
		  pos++;
	  }
  }

  void LCD_DisplayVel(double vel)
  {
	  unsigned int pos = nr_cif((int)(vel)) + 2 + 5;

	  LCD_SetPosition(2);
	  LCD_WriteDouble(vel);

	  LCD_SetPosition(pos);
	  while (pos < 12)
	  {
		  LCD_WriteData(' ');
		  pos++;
	  }
  }

 void LCD_DisplayPos(int pos)
 {
	unsigned int pos1 = nr_cif((int)(pos)) + 16;

	LCD_SetPosition(16);
	LCD_WriteInt(pos);

	LCD_SetPosition(pos1);
	while (pos1 < 20)
	{
		LCD_WriteData(' ');
		pos1++;
	}
 }

 void LCD_DisplayRef(int ref)
 {
	unsigned int pos = nr_cif((int)(ref)) + 36;
	
	LCD_SetPosition(36);
	LCD_WriteInt(ref);

	LCD_SetPosition(pos);
	while (pos < 40)
	{
		LCD_WriteData(' ');
		pos++;
	}
 }