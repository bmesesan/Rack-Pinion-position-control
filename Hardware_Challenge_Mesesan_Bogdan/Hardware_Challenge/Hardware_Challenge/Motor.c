/*
 * Motor.c
 *
 * Created: 3/24/2020 3:08:56 PM
 *  Author: Bogdan
 */ 
 #include "Motor.h"

void setupPorts(void)
{
	//LCD ports
	//RS and R/W
	DDRC |= (1 << DDC0 | 1 << DDC1);
	//D7, D6, D5, D4
	DDRC |= (1 << DDC2 | 1 << DDC3 | 1 << DDC4 | 1 << DDC5);
	//EN
	DDRD |= (1 << DDD4);

	//Timer0 port
	DDRD |= (1 << DDD5);

	//Motor direction control
	DDRD |= (1 << DDD6 | 1 << DDD7);

	//Interrupts input ports
	DDRD &= ~(1 << DDD2);
	DDRD &= ~(1 << DDD3);

	//Buttons input pins
	DDRB = ~(1 << DDB0 | 1 << DDB1 | 1 << DDB2);
}


//Timer0 is used to generate the PWM signal which drives the motor
//The duty factor is between 0 and 255
 void setupTimer0(void)
 {
	//Set Timer0 in Fast PWM mode, inverting type
	 TCCR0A |= (1 << COM0B1 | 1 << COM0B0 | 1 << WGM01 | 1 << WGM00);
	 TCCR0B |= (1 << WGM02);
	 //OCR0A will set the Maximum PWM
	 OCR0A = MAX_PWM;
	 //OCR0B will set the duty cycle
	 OCR0B = MAX_PWM;
 }

 void startTimer0(void)
 {
	 //Prescaler N = 256
	 TCCR0B |= (1 << CS02 | 0 << CS01 | 0 << CS00);
 }

 void stopTimer0(void)
 {
	 TCCR0B &= !(1 << CS02 | 0 << CS01 | 0 << CS00);
 }


 //Timer1 is programmed to generate an interrupt at every 10 microseconds
 //I use this value to calculate speed and acceleration
 void setupTimer1(void)
 {
	//Set CTC mode with compare set on OCR1A
	 TCCR1B |= (1 << WGM12);
	 OCR1AH = 0X00;
	 OCR1AL = 159;
 }

//Activate Timer1 interrupt on compare with OCR1A
 void setupTimer1Int(void)
 {
	 TIMSK1 |= (1 << OCIE1A);
 }

 void startTimer1(void)
 {
	//Prescaler N = 1
	 TCCR1B |= (0 << CS12 | 0 << CS11 | 1 << CS10);
 }

 void stopTimer1(void)
 {
	 TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);
 }

 //Int0 and Int1 are used for the signals coming from the encoder
 //They are active on the Rising edge
 void setupInt0(void)
 {
	 DDRD &= ~(1 << DDB2);
	 EICRA |= (1 << ISC01 | 1 << ISC00);
	 EIMSK |= (1 << INT0);
 }

 void setupInt1(void)
 {
	 DDRD &= ~(1 << DDB3);
	 EICRA |= (1 << ISC11 | 1 << ISC10);
	 EIMSK |= (1 << INT1);
 }

 //Pin Change Interrupt on PortB, used for the user interface buttons
 void setupPinChgInt(void)
 {
	PCMSK0 |= (1 << PCINT0 | 1 << PCINT1 | 1 << PCINT2);
	PCICR |= (1 << PCIE0);
 }

 
 void MotorSetup(void)
 {
	 MotorSetDirection(0);
	 setupTimer1();
	 setupTimer1Int();
	 setupTimer0();
	 setupInt0();
	 setupInt1();
	 setupPinChgInt();
	 sei();
 }

 void MotorSetDuty(unsigned char duty)
 {
	//This operation is needed because the Timer is set inverting mode
	 OCR0B = MAX_PWM - duty;
 }

 void MotorSetDirection(unsigned char dir)
 {
	 if (dir == 1)
	 {
		 PORTD &= ~(1 << PORTD6 | 1 << PORTD7);
		 PORTD |= (1 << PORTD6);
	 }
	 else
	 {
		 if (dir == 0)
		 {
			 PORTD &= ~(1 << PORTD6 | 1 << PORTD7);
			 PORTD |= (1 << PORTB7);
		 }
	 }
 }

 void MotorStop(void)
 {
	MotorSetDuty(0);
 }

 //get the necessary direction based on the reference position and the actual position
unsigned int getDirection(int ref, int pos)
 {
	if (ref > pos)
		return (1);
	else
		return (0);
 }


 //Get the error = Set_Point - Process_Variable
 int getError(int sp, int pv)
 {
	int aux = sp * PINION_CONSTANT;

	if (sp > RACK_TEETH)
		aux = MAX_POSITION;
	if (sp < 0)
		aux = 0;

	return (aux - pv);
 }

 //Get the necessary PID drive
 int getPIDdrive(int error, int prev_error, double *errorI)
 {
	double aux_errorI = *errorI;
	double drvP, drvI, drvD, drvPID;

	//Proportional Drive
	drvP = (double)(error * KP);

	//the Integral Drive will have a contribution only when the error is not very large
	//The integral will control the DC error
	if (aux_errorI < IMAX_LIM && aux_errorI > -IMAX_LIM && error != 0)
	{
		aux_errorI += error;
	}
	else
	{
		aux_errorI = 0;
	}
	//The integral error is limited
	if (aux_errorI > (double)(5 / KI))
	{
		aux_errorI = (double)(5 / KI);
	}
	if (aux_errorI < -(double)(5 / KI))
	{
		aux_errorI = -(double)(5 / KI);
	}
	*errorI = aux_errorI;
	//Integral Drive
	drvI = (double)(KI * aux_errorI);

	//if the error is 0, then the derivative drive will not have a contribution
	if (error == 0)
	{
		drvD = 0;
	}
	else
	{
		//Derivative Drive, acting on past errors
		drvD = (double)((error - prev_error) * KD);
	}

	//The final necessary drive
	drvPID = drvP + drvI + drvD;
	if (drvPID < 0)
		drvPID *= -1;
	if ((int)(drvPID) > MAX_PWM)
		 drvPID = MAX_PWM;

	return ((int)(drvPID));
 }