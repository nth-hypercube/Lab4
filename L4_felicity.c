/*----------------------------------------------------------------------------
LAB 4
Felicity Kubic, Melody Fan
4-3-18
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h> // Include files. This file is available online in LMS
#include <i2c.h>        // Get from LMS, THIS MUST BE INCLUDED AFTER stdio.h
#define PCA_START 28672 // 28672 for exactly 20ms

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);   // Initialize ports for input and output
void Interrupt_Init(void);
void PCA_Init(void);
void SMB0_Init(void);
void PCA_ISR(void) __interrupt 9;

//initial values
void initialize(void);


//Control functions
void PingRanger(void);
unsigned int ReadRanger(void);
void Drive_Motor(void);

unsigned int ReadCompass(void);
void


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------

void main(void)
{
    // initialize board
    Sys_Init();
    putchar(' '); //the quotes in this line may not format correctly
    Port_Init();
    XBR0_Init();
    PCA_Init();
	Timer_Init();

	Counts = 0;
    while (Counts < 1); // Wait a long time (1s) for keypad & LCD to initialize

	//Print Start Message
	 printf("\r\n=====Start=====\r\n");





}


//-----------------------------------------------------------------------------
// Initializations
//-----------------------------------------------------------------------------
void Port_Init()
{
    P1MDOUT |= 0xF; // set CEX pins to push-pull
	P3MDOUT &= 0x3F; // set slide switches to open drain
	P3 |= ~0x3F; // set slide switches to high impedance

    XBR0 = 0x27;

void PCA_Init(void)
{
    PCA0MD = 0x81;   // SYSCLK/12, enable CF interrupts, suspend when idle
    PCA0CPM3 = 0xC2; // 16 bit, enable compare, enable PWM
    PCA0CN |= 0x40;  // enable PCA
	EIE1 |= 0x08;	//Enable PCA interrupts
	EA |= 1;			//enable all interrupts
}

void SMB0_Init()
{
	SMB0CR = 0x93;	//set SCL to 100 kHz
	ENSMB = TRUE;	//enable SMBUS0
}

void ADC_Init(void)
{
  	REF0CN = 0x03;  // Configure ADC1 to use Vref
  	//REF0CN &= 0xF7;
  	ADC1CF |= 0x01; // Set gain = 1
  	ADC1CN = 0x80;  // Enable ADC1
}


void PCA_ISR(void) __interrupt 9
{
    if (CF)
	{
	    CF = 0;             // clear interrupt flag
       	PCA0 = 	0x7000;		// set period to 20 ms

		h_count ++;
		if(h_count>2)
		{
			new_heading = 1; // 2 overflows is about 40 ms
			h_count = 0;
		}
	}
	PCA0CN &= 0xC0;          // all other type 9 interrupts
}

unsigned int ADC_result(void)
{
  	AMX1SL = 1; // set port number
  	ADC1CN = ADC1CN & ~0x20; // clear flag from prev conversion
  	ADC1CN = ADC1CN |= 0x10; // start an ADC conversion
  	while ( (ADC1CN & 0x20) == 0x00); // wait while still converting
  	return ADC1;		//ADC1 is the conversion rate
}


//-----------------------------------------------------------------------------
// Function Deifnitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
void Drive_Motor(void)
{
	char input;
	//wait for keyborad input
	input = getchar();
	if(input == 'f')
	{
		if(MOTOR_PW < PW_MAX)
		MOTOR_PW = MOTOR_PW + 10; // increse steering PW by 10
		PCA0CP2 = 65536 - MOTOR_PW;  // change pulse width
	}
	else if(input == 's')
	{
		if(MOTOR_PW > PW_MIN)
		MOTOR_PW = MOTOR_PW - 10;
		PCA0CP2 = 65536 - MOTOR_PW;  // change pulse width
	}

}


//Get initial values
void initialize(void)
{
	lcd_clear();
	//Get heading
	printf("\r\nEnter heading on keypad\r\n 1 = 0 deg \r\n 2 = 90 deg \r\n 3 = 180 deg \r\n 4 = 270 deg ");
	while(read_keypad() == 0xFF) pause();
		if(read_keypad() != 0xFF)   // keypad = 0xFF if no key is pressed
        {
			lcd_clear();
			if(keypad == 1)
			if(keypad == 2)
			if(keypad == 3)
			if(keypad == 4)

        }

	//Get Steering Gain
	printf("\r\nEnter steering gain kp\r\n 1 = 0.5 \r\n 2 = 0.75 \r\n 3 = 1.5 \r\n 4 = 2 ");
	while(read_keypad() == 0xFF) pause();
		if(read_keypad() != 0xFF)   // keypad = 0xFF if no key is pressed
        {
			lcd_clear();
			if(keypad == 1)
			if(keypad == 2)
			if(keypad == 3)
			if(keypad == 4)

        }
}

void pause(void)
{
    nCounts = 0;
    while (nCounts < 1);// 1 count -> (65536-PCA_START) x 12/22118400 = 20ms
}                       // 6 counts avoids most of the repeated hits
