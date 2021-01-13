/* clock.s
 *
 * Last Updated: 19 Nov 2020
 * Author: Wes, Dr. Schwartz
 */ 

 #include <avr/io.h>

 .section .text

 .global clock_init
 clock_init:
	push r24

	ldi r24, OSC_RC32MEN_bm
	sts OSC_CTRL, R24		;Enables the 32MHz internal oscillator

check32MHzStatus:
	lds	r24, OSC_STATUS
	;Ensure that the 32MHz clock is ready before proceeding
	sbrs r24, OSC_RC32MRDY_bp
	rjmp check32MHzStatus

	;Writing to CCP disables interrupts for a certain number of cycles
	;to give the clock time to switch sources. It also enables writes to certain registers.
	ldi r24, 0xD8
	sts CPU_CCP, r24				

	;Finally, select the now-ready 32MHz oscillator as the new clock source. 
	ldi	r24, 0x01
	sts CLK_CTRL, r24

skip32MHZ_enable:
	;CPU CLK prescaler settings 
	;Use values that are powers of 2 from 1 to 512 (1, 2, 4, 8, 16, ..., 512) for A. See Table 7-2 in the manual. 
	;You can also change B/C. See Table 7-3 in the manual. 

	ldi r24, 0xD8
	sts CPU_CCP, r24
	
	ldi r24, ((0x00 << 2) | (0x00 << 0))	;32MHz
	;ldi r24, (CLK_PSADIV_4_gc | CLK_PSBCDIV_1_1_gc) ;8MHz
	;ldi r24, 0b00010100 ; 4Mhz
	sts CLK_PSCTRL, r24

	pop r24

	ret