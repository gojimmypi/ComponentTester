/* ************************************************************************
 *
 *   wait functions
 *
 *   based on code from Karl-Heinz K�bbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define WAIT_S


/*
 *  includes
 */

/* basic includes */
#include <avr/io.h>

/* local includes */
#include "config.h"           /* global configuration */



.func wait1000ms
.global wait5s		; wait 5 seconds
.global wait4s		; wait 4 seconds
.global wait3s		; wait 3 seconds
.global wait2s		; wait 2 seconds
.global wait1s		; wait 1 seconds
.global wait1000ms	; wait 2 second and wait 1000ms are identical
.global wait500ms	; wait 500ms
.global wait400ms	; wait 400ms
.global wait300ms	; wait 300ms
.global wait200ms	; wait 200ms
.global wait100ms	; wait 100ms
.global wait50ms	; wait 50ms
.global wait40ms	; wait 40ms
.global wait30ms	; wait 30ms
.global wait20ms	; wait 20ms
.global wait10ms	; wait 10ms
.global wait5ms		; wait 5ms
.global wait4ms		; wait 4ms
.global wait3ms		; wait 3ms
.global wait2ms		; wait 2ms
.global wait1ms		; wait 1ms
.global wait500us	; wait 500µs
.global wait400us	; wait 400µs
.global wait300us	; wait 300µs
.global wait200us	; wait 200µs
.global wait100us	; wait 100µs
.global wait50us	; wait 50µs
.global wait40us	; wait 40µs
.global wait30us	; wait 30µs
.global wait20us	; wait 20µs
.global wait10us	; wait 10µs

; waiting functions for MCU clock rates of 1MHz, 2MHz, 4MHz, 8MHz and 16MHz
; stacked design to minimize flash memory usage:
; - 68 Byte at 1MHz
; - 70 Byte at 2MHz
; - 76 Byte at 4MHz
; - 74 Byte at 8Mhz
; No registers are used, just the stack pointer.
; Maximum RAM usage of 28 bytes for return addresses.


/*
 *  20�s and up (clock rate independent)
 */

wait5s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait4s	;+4s

wait4s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait3s	;+3s

wait3s:
 rcall wait1s		;1s	12+x return addresses
;proceed with wait2s	;+2s

wait2s:
 rcall wait1s		;1s     12+x return addresses
;proceed with wait1s	;+1s

wait1s:
wait1000ms:	
 rcall wait500ms	;500ms	11+x return addresses
;proceed with wait500ms	;+500ms	

wait500ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait400ms	;+400ms

wait400ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait300ms	;+300ms

wait300ms:
 rcall wait100ms        ;100ms	10+x return addresses
;proceed with wait200ms	;+200ms

wait200ms:
 rcall wait100ms	;100ms	10+x return addresses
;proceed with wait100ms ;+100ms

wait100ms:
 wdr			;reset watchdog every 100ms!
 rcall wait50ms	    	;50ms	9+x return addresses
;proceed with wait50ms 	;+50ms

wait50ms:
 rcall wait10ms	    	;10ms	8+x return addresses 
;proceed with wait40ms	;+40ms

wait40ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait30us 	;+30ms

wait30ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait20us 	;+20ms

wait20ms:
 rcall wait10ms	    	;10ms	8+x return addresses
;proceed with wait10ms	;+10ms

wait10ms:
 rcall wait5ms	   	;5ms	7+x return addresses
;proceed with wait5ms	;+5ms

wait5ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait4ms	;+4ms

wait4ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait3ms	;+3ms	

wait3ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait2ms	;+2ms

wait2ms:
 rcall wait1ms	   	;1ms	6+x return addresses
;proceed with wait1ms	;+1ms

wait1ms:
 rcall wait500us    	;500�s	5+x return addresses
;laeuft in wait500us	;+500�s

wait500us:
 rcall wait100us    	;100�s	4+x return addresses
;proceed with wait400us	;+400�s

wait400us:
 rcall wait100us    	;100�s	4+x return addresses
;proceed with wait300us	;+300�s

wait300us:
 rcall wait100us    	;100�s	4+x return addresses
;proceed with wait200us	;+200�s

wait200us:
 rcall wait100us    	;100�s	4+x return addresses
;proceed with wait100us	;+100�s	

wait100us:
 rcall wait50us	    	;50�s	3+x return addresses
;proceed with wait50us	;+50�s	

wait50us:
 rcall wait10us     	;10�s	2+x return addresses
;proceed with wait40us	;+40�s

wait40us:
 rcall wait10us     	;10�s	2+x return addresses
;proceed with wait30us	;+30�s

wait30us:
 rcall wait10us     	;10�s	2+x return addresses
;proceed with wait20us	;+20�s

wait20us:
 rcall wait10us		;10�s	2+x return addresses
;proceed with wait10us	;+10�s


/*
 *  10�s and below based on MCU clock rate
 */

wait10us:		;	1+x return addresses

#if CPU_FREQ == 1000000
;1MHz version			x = 0
;rcall needs 3 clock cycles for ATmega8L
 nop			;if call needs 4 clock cycles, remove one nop!
 nop
 nop
;ret needs 4 clock cycles (1�s each)
#endif


#if CPU_FREQ == 2000000
;2MHz version			x = 1
 rcall wait5us		;5s
;proceed with wait5us	;+5�

.global wait5us		;wait 5�s
wait5us:
 nop
 nop
 nop
#endif


#if CPU_FREQ == 4000000
;4MHz version			x = 2
.global wait5us		;wait 5�s
.global wait4us		;wait 4�s
.global wait2us		;wait 2�s
 rcall wait5us		;5�s
;proceed with wait5us	;+5�s

wait5us:
 nop
 nop
 nop
 nop			;1�s
;proceed with wait4us	;+4�s

wait4us:
rcall wait2us		;2�s
;proceed with wait2us	;+2�s

wait2us:
 nop			;2�s
#endif


#if CPU_FREQ == 8000000
;8MHz Version			x = 2
.global wait5us		;wait 5�s
.global wait4us		;wait 4�s
.global wait3us		;wait 3�s
.global wait2us		;wait 2�s
.global wait1us		;wait 1�s
 rcall wait5us		;5�s
;proceed with wait5us	;+5�s

wait5us:
 rcall wait1us		;1�s
;proceed with wait4us	;+4�s

wait4us:
 rcall wait1us		;1�s
;proceed with wait3us	;+3us

wait3us:
 rcall wait1us		;1�s
;proceed with wait2us	;+2�s

wait2us:
 rcall wait1us		;1�s
;proceed with wait1us	;+1�s

wait1us:
 nop			;1�s
#endif


#if CPU_FREQ == 16000000
;16MHz Version			x = 2
.global wait5us		;wait 5�s
.global wait4us		;wait 4�s
.global wait3us		;wait 3�s
.global wait2us		;wait 2�s
.global wait1us		;wait 1�s
 rcall wait5us		;5�s
;proceed with wait5us	;+5�s

wait5us:
 rcall wait1us		;1�s
;proceed with wait4us	;+4�s

wait4us:
 rcall wait1us		;1�s
;proceed with wait3us	;+3�s

wait3us:
 rcall wait1us		;1�s
;proceed with wait1us	;+2�s

wait2us:
 rcall wait1us		;1�s
;proceed with wait1us	;+1�s

wait1us:
 rcall wait500ns	;500ns
;proceed with wait500ms	;+500ns

wait500ns:
 nop			;500ns
#endif


 ret



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef WAIT_S



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
