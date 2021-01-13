#include "joystick.h"

#define PER_ADC ((2000000/64)/100)

void joystick_adc_init(void)
{
	// Signed, 12-bit conversion, right adjusted, Normal mode
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm;
	
	// 2.5V reference voltage on PORTB pin 0
	ADCA_REFCTRL = ADC_REFSEL_AREFB_gc;
	
	// Set prescaler to 512
	ADCA.PRESCALER = ADC_PRESCALER_DIV32_gc;
	
	// Single ended - x axis
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	// Single ended - y axis
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
	
	// Measure voltage on PORTA pin 0
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;
	
	// Measure voltage on PORTA pin 1
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;
	
	// Cause low-level interrupt on CH0 when conversion is complete
	ADCA.CH0.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;
	
	// Cause low-level interrupt on CH0 when conversion is complete
	ADCA.CH1.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;
	
	// Event channel 0 to cause ADC to start conversion on ADCA.CH0 and ADCA.CH1
	ADCA.EVCTRL = ADC_SWEEP_01_gc | ADC_EVSEL_0123_gc | ADC_EVACT_SWEEP_gc;
	
	// Enable ADC
	ADCA.CTRLA = ADC_ENABLE_bm;
}

void joystick_tcc1_init(void)
{
	TCC1.PER = PER_ADC;
	
	// TCC0 overflow to cause event on channel 0
	EVSYS.CH0MUX = EVSYS_CHMUX_TCC1_OVF_gc;
	
	// Initialize TCC0 with prescaler 64
	TCC1.CTRLA = TC_CLKSEL_DIV64_gc;
}