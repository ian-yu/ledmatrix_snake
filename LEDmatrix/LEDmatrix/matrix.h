/*
 * matrix.h
 *
 * Created: 12/16/2020 6:41:05 PM
 *  Author: ianyu
 */ 


#ifndef MATRIX_H_
#define MATRIX_H_

#include <avr/io.h>
#include <avr/interrupt.h>


// Initializes appriopriate PORT pins
void matrix_init(void);

// Turns on the specified column to the input row
// col: 0 - 7
// row: 8 - bit binary value of the output row with MSB representing 7th column on LSB representing 0th column   
void matrix_turnon(uint8_t row, uint8_t col);

// Initializes TCC0 timer and interrupt to refresh matrix
void refreshmatrix_init(void);

#endif /* MATRIX_H_ */