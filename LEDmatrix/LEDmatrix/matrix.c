/*
 * matrix.c
 *
 * Created: 12/16/2020 6:41:21 PM
 *  Author: ianyu
 */ 

#include "matrix.h"

#define PORTF_col_bm (0b11010110)
#define PORTF_row_bm (0b00101001)
#define PORTC_col_bm (0b00110100)
#define PORTC_row_bm (0b11001011)

void matrix_init(void)
{
	// Sets all columns to 1 and rows to 0
	PORTC.OUT = PORTC_col_bm;
	PORTF.OUT = PORTF_col_bm;
	
	PORTC.DIRSET = 0xFF;
	PORTF.DIRSET = 0xFF;	
}

void matrix_turnon(uint8_t row, uint8_t col)
{
	// Sets all columns to 1 and rows to 0
	PORTC.OUT = PORTC_col_bm;
	PORTF.OUT = PORTF_col_bm;
		
	if (col == 0)
	{
		PORTF.OUTCLR = (1<<4);
	}
	else if (col == 1)
	{
		PORTC.OUTCLR = (1<<5); 
	}
	else if (col == 2)
	{
		PORTC.OUTCLR = (1<<4);
	}
	else if (col == 3)
	{
		PORTF.OUTCLR = (1<<1);
	}
	else if (col == 4)
	{
		PORTC.OUTCLR = (1<<2);
	}
	else if (col == 5)
	{
		PORTF.OUTCLR = (1<<2);
	}
	else if (col == 6)
	{
		PORTF.OUTCLR = (1<<6);
	}
	else if (col == 7)
	{
		PORTF.OUTCLR = (1<<7);
	}
	if (row & (1<<7))
	{
		PORTC.OUTSET = (1<<3);
	}
	if (row & (1<<6))
	{
		PORTC.OUTSET = (1<<6);
	}
	if (row & (1<<5))
	{
		PORTC.OUTSET = (1<<1);
	}
	if (row & (1<<4))
	{
		PORTC.OUTSET = (1<<7);
	}
	if (row & (1<<3))
	{
		PORTF.OUTSET = (1<<3);
	}
	if (row & (1<<2))
	{
		PORTC.OUTSET = (1<<0);
	}
	if (row & (1<<1))
	{
		PORTF.OUTSET = (1<<5);
	}
	if (row & (1<<0))
	{
		PORTF.OUTSET = (1<<0);
	}
}

// Initializes TCC0 interrupts for refresh matrix display
void refreshmatrix_init(void)
{
	TCC0.PER = 20;
	TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
}