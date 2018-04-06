#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>

#define PCA_START 28672 // 28672 for exactly 20ms


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
unsigned int PW_CETNER = ;
unsigned int PW_RIGHT = ;
unsigned int PW_LEFT = ;
unsigned int SERVO_PW = ;
unsigned int SERVO_MAX = ;
unsigned int SERVO_MIN = ;

unsigned int heading, range;
unsigned int desired_h, desired_r;

unsigned char new_heading = new_range = print_flag = 0; // count flag for compass timing, ranger timing, printing
unsigned char h_count, r_count, print_count;            // overflow count for heading, range, printing

int compass_adj = range_adj = 0; // correction to steering direction and speed

__sbit __at 0xB6 POT; // potentiameter on P3.6
__sbit __at 0xB7 SS;  // slide switch on P3.7

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void); // includes XBAR init
void PCA_Init (void); // includes interrupt init
void SMB0_Init(void);
void ADC_Init(void);
void PCA_ISR (void) __interrupt 9;

int ReadCompass(void);
void Set_Servo(void);

int ReadRanger(void); // read ranger and then ping
int pick_heading(void); // ask operator to pick heading
int pick_range(void);

unsigned int ADC_result(unsigned int n);      // Returns ADC1 conversion result, n = pin number

//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    unsigned char run_stop = 0;
	// initialize board
    Sys_Init();
    putchar(' ');
    Port_Init();
	PCA_Init ();
	SMB0_Init();
	ADC_Init();

    // print instructions and ask for settings
    printf("\r\n ...\r\n");


    while(1)
	{
		run_stop = 0;
		while (!SS)
		{
			if (run_stop == 0)
			{

			//ask user to select desired heading, 0 = 0, 1 = 90, 2 = 180, 3 = 270
			desired_h = pick_heading();
			desired_r = pick_range();
			//display heading on LCD
			//ask user to set gain
			//display gain on LCD
			//display current heading and range (*and battery voltage)
			//update every 400ms

			//collect and store Data

			} // end if run_stop

		} // end while !SS
	} // end while 1

} // end main code

//-----------------------------------------------------------------------------
// Function Deifnitions
//-----------------------------------------------------------------------------
void Port_Init()
{
    P1MDOUT |= 0x0F; // set CEX pins to push-pull
	P3MDOUT &= 0x3F; // set slide switches to open drain              NEED DETERMINE PORT 1 CONFIGURATIONS
	P3 |= ~0x3F; // set slide switches to high impedance

    XBR0 = 0x27;

void PCA_Init(void)
{
    PCA0MD = 0x81;   // SYSCLK/12, enable CF interrupts, suspend when idle
    PCA0CPM3 = 0xC2; // 16 bit, enable compare, enable PWM
    PCA0CN |= 0x40;  // enable PCA
	EIE1 |= 0x08;	 //Enable PCA interrupts
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
  	ADC1CF |= 0x01; // Set gain = 1
  	ADC1CN = 0x80;  // Enable ADC1
}


void PCA_ISR(void) __interrupt 9
{
    if (CF)
	{
	    CF = 0;             // Very important - clear interrupt flag
       	PCA0 = 	36864;		// start count
		h_count ++;
		if(h_count>2)
		{
			// ADD DIFFERENT VARIABLES HERE TO CONTROL EACH UPDATE INTERVALS

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


void Set_Servo()
{
    if (new_heading) // enough overflows for a new heading
	{
		heading = ReadCompass();

		printf("\r\n%u\r\n",heading);

		// STEER ADJ = DESIRED - HEADING
		// RANGE ADJ = IN_RANGE - RANGE
		// STEER_PW = NEUTRAL PW + STEER ADJ + RANGE ADJ


		if ((error < -1800) || (error > 1800))
		{
			error = 1800 - desired - heading;
			PW = ((unsigned int) (k*error + PW_CENTER));
			PCA0CP0 = 0xFFFF - PW;
		}

		else
		{	PW = ((unsigned int) (k*error + PW_CENTER));
			PCA0CP0 = 0xFFFF - PW;
		}
		new_heading = 0;
	}
}

void PingRanger()
{
	unsigned char Data[2];
	unsigned int range = 0;
	unsigned char addr=0xE0;	//Address of the ranger
	Data[0] = 0x51;
	i2c_write_data(addr, 0, Data, 1);
}

unsigned int ReadRanger()
{
	unsigned char Data[2];
	unsigned int range = 0;
	unsigned char addr=0xE0;	//Address of the ranger
	i2c_read_data(addr, 2, Data, 2);
	range = (((unsigned int)Data[0] << 8) | Data[1]);
	return range;
}

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
            {
              desired_h = 0;
            }
      			if(keypad == 2)
            {
              desired_h = 900;
            }
      			if(keypad == 3)
            {
              desired_h = 1800;
            }
      			if(keypad == 4)
            {
              desired_h = 2700;
            }
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
