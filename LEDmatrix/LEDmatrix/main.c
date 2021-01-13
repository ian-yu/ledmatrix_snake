#include <avr/io.h>
#include <avr/interrupt.h>
#include "matrix.h"
#include "joystick.h"

volatile uint8_t led_matrix[8];
volatile uint8_t row_count = 0;
volatile uint8_t conversion_x_flag = 0;
volatile uint8_t conversion_y_flag = 0;
volatile int16_t x_axis;
volatile int16_t y_axis;
uint8_t xpos = 0;
uint8_t ypos = 0;


const uint8_t zero_matrix[8] =
{
	0b01111100,
	0b01000100,
	0b01000100,
	0b01000100,
	0b01000100,
	0b01000100,
	0b01000100,
	0b01111100
};

const uint8_t one_matrix[8] = 
{
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100				
};

const uint8_t two_matrix[8] = 
{
	0b00111100,
	0b00000100,
	0b00000100,
	0b00111100,
	0b00100000,
	0b00100000,
	0b00100000,
	0b00111100
};

void delay(void);
void update_dot(void);

int main(void)
{
	// Initialize all values to 0
	for (uint8_t i = 0; i < 8; i++)
	{
		led_matrix[i] = 0;
	}
	
	// Initialize Delay
	TCD0.PER = 200;

	matrix_init();	
	refreshmatrix_init();
	joystick_adc_init();
	joystick_tcc1_init();

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

	while (1)
	{
		if (conversion_x_flag || conversion_y_flag)
		{
			update_dot();
		}
	}
}

// Generic delay
void delay()
{
	TCD0.CTRLA = TC_CLKSEL_DIV1024_gc;
	while(!(TCD0.INTFLAGS & TC0_OVFIF_bm));
	TCD0.INTFLAGS = TC0_OVFIF_bm;
}

// Update LED Matrix based on ADC location
void update_dot()
{
	// Left
	if (x_axis > 1200)
	{
		conversion_x_flag = 0;
		xpos = (xpos+1)%8;
	}
	// Right
	else if (x_axis < 800)
	{
		conversion_x_flag = 0;
		if (xpos == 0)
		xpos = 7;
		else
		xpos--;
	}
	// Up
	if (y_axis > 1200)
	{
		conversion_y_flag = 0;
		if (ypos == 0)
			ypos = 7;
		else
			ypos--;
	}
	// Down
	else if (y_axis < 800)
	{
		conversion_y_flag = 0;
		ypos = (ypos+1)%8;
	}
	for (uint8_t i = 0; i < 8; i++)
	{
		if (i == ypos)
		{
			led_matrix[ypos] = (1<<xpos);
		}
		else
		{
			led_matrix[i] = 0;
		}
	}
	delay();
}

// Refresh matrix
ISR(TCC0_OVF_vect)
{
	// Displays next row
	matrix_turnon(led_matrix[row_count],row_count);
	row_count++;
	if (row_count == 8)
	{
		row_count = 0;
	}
}

// X axis ADC
ISR(ADCA_CH0_vect)
{
	x_axis = ADCA.CH0.RES;
	conversion_x_flag = 1;
}

// Y axis ADC
ISR(ADCA_CH1_vect)
{
	y_axis = ADCA.CH1.RES;
	conversion_y_flag = 1;
}

/*
		for (uint8_t i = 0; i < 3; i++)
		{
			for (uint8_t j = 0; j < 8; j++)
			{
				if (i == 0)
				{
					led_matrix[j] = zero_matrix[j];
				}
				else if (i == 1)
				{
					led_matrix[j] = one_matrix[j];
				}
				else if (i == 2)
				{
					led_matrix[j] = two_matrix[j];
				}
			}
			delay();
		}
		
*/

/*
		for (uint8_t i = 0; i < 8; i++)
		{
			for (uint8_t j = 0; j < 8; j++)
			{
				led_matrix[i] = (1<<j);
				delay();
				if (j == 7)
				{
					led_matrix[i] = 0;
				}
			}
		}
*/