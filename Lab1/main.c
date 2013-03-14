/* Lab1 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 3/9/2013 2:23:10 PM
 *  Author: Kyle
 */

/*
** Refer to the ATmega1284P datasheet (doc8272 rev 05/12)
**   -  8-bit timer: pages 104-110
**   - 16-bit timer: pages 132-135
*/

/*
** Assignment (general) - Use three different ways to toggle separate LEDs by using timers
**   1. 1ms timer   -> schedule task 1
**   2. 100ms timer -> schedule task 2
**   3. PWM signal   -> schedule task 3
*/

#include <pololu/orangutan.h>
#include <avr/io.h>         //gives us names for registers
#include <avr/interrupt.h>
#include "timer_1284p.h"

static int ms_tick;
static int release;

int main()
{
    cli();
    ms_tick = 0;
    release = 0;

    // Set up IO
    
    
    // Set up timers
    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescalor ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 1 ms (1000Hz) and need to find values of prescalor and timer_period (TOP, aka OCRnA)
    **
    ** 1000 = (20E6 / 1) * (1 / prescalor) * (1 / timer_period)
    ** prescalor = 20E6 / (1000 * timer_period)
    **
    ** prescalor = 256
    ** timer_period = 78
    */

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescalor of 256
    timer_1284p_set_COM( TIMER_1284P_0, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_0, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_0, TIMER_1284P_CS_PRESCALE_DIV256);

    // Timer period of 78 (8-bit register)
    timer_1284p_set_OCR( TIMER_1284P_0, TIMER_1284P_A, 78 - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_0, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_OVERFLOW );

    clear();

    //Global interrupt enable
    sei();

    int red_LED_value = 0;
    int timer_dbc = 0;
    while( 1 )
    {
        lcd_goto_xy(0, 0);
        print_long(TCNT0);
        if ( release == 1 )
        {
            //Toggle red
            red_led(red_LED_value);
            red_LED_value ^= 0x1;
            release = 0;
        }
	}
}

ISR(TIMER0_COMPA_vect)
{
    char cSREG;
    
    cSREG = SREG;
    
    ms_tick++;
    if ( ms_tick == 1000 )
    {
        ms_tick = 0;
        release = 1;
    }
    
    SREG = cSREG;
}
