/*
 * Test1.c
 *
 * Created: 3/22/2020 12:29:54 PM
 * Author : Bogdan
 */ 

#include "LCD.h"
#include "Motor.h"
#include <math.h>
#include <stdlib.h>

volatile int motorPosition = 0;
volatile int pinionPosition = 0;
volatile double velocity = 0;
volatile double prev_velocity = 0;
volatile double acceleration = 0;
volatile unsigned int count10us = 1;
volatile int refPosition = 0;
volatile int refPosition_select = 0;

int main(void)
{
	setupPorts();
	LCD_Setup();
	LCD_InitialDisplay();
	MotorSetup();

	int error = 0;
	int prev_error = 0;
	double errorI = 0;
	int drvPID = 0;

    while (1) 
    {	
		prev_error = error;
		error = getError(refPosition, motorPosition);
		MotorSetDirection(getDirection(refPosition, pinionPosition));
		drvPID = getPIDdrive(error, prev_error, &errorI);
		MotorSetDuty(drvPID);
		LCD_DisplayVel(velocity);
		LCD_DisplayAcc(acceleration);
		LCD_DisplayPos(pinionPosition);
		LCD_DisplayRef(refPosition_select);
    }
}

//Pin change interrupt 0, on PORTB0, PORTB1, PORTB2
ISR (PCINT0_vect)
{
	//REF+ is pressed
	if (!(PINB & (1 << PINB0)))
	{
		//Limit the reference to the maximum position of the pinion
		if (refPosition_select == RACK_TEETH)
		{
			refPosition_select = RACK_TEETH;
		}
		else
		{
			refPosition_select++;
		}
	}
	//REF- is pressed
	if (!(PINB & (1 << PINB1)))
	{
		//Limit to the minimum position
		if (refPosition_select == 0)
		{
			refPosition_select = 0;
		}
		else
		{
			refPosition_select--;
		}
	}
	//The desired position is confirmed
	if (!(PINB & (1 << PINB2)))
	{
		refPosition = refPosition_select;
		startTimer0();
	}
}

//Timer1 interrupt, generated at every 10 us
ISR (TIMER1_COMPA_vect)
{
	count10us++;
}

//External interrupt 0
ISR (INT0_vect)
{
	//if a rising edge is detected when the other signal is low means a Motor position increment
	if (!(PIND & (1 << PIND3)))
	{
		stopTimer1();
		motorPosition++;
		//set the Pinion position
		pinionPosition = motorPosition / PINION_CONSTANT;
		//calculates the velocity based on the rate of change in position
		prev_velocity = velocity;
		velocity = (double)(VELOCITY_CONST / count10us);
		//calculates the acceleration, based on the current velocity and and previous velocity
		acceleration = (double)(((velocity * 100000 - prev_velocity * 100000) / count10us));
		//reset the counter
		count10us = 0;
		startTimer1();
	}
}

//External interrupt 1
ISR (INT1_vect)
{
	//if a rising edge is detected when the other signal is low means a Motor position decrement
	if (!(PIND & (1 << PIND2)))
	{
		stopTimer1();
		motorPosition--;
		//set the Pinion position
		pinionPosition = motorPosition / PINION_CONSTANT;
	//calculates the velocity based on the rate of change in position
		prev_velocity = velocity;
		velocity = (double)(VELOCITY_CONST / count10us);
		//calculates the acceleration, based on the current velocity and and previous velocity
		acceleration = (double)(((velocity * 100000 - prev_velocity * 100000) / count10us));
		//reset the counter
		count10us = 0;
		startTimer1();
	}
}