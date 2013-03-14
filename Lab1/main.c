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

#define CPU_FREQ 20000000
#define TICKS_PER_CYCLE 4
#define MS_PER_S 1000
#define NUM_MS_TO_DELAY 10
// ( 20M CPU cycles / 1 second ) * ( 1 second / 1000 milliseconds ) * ( 1 loop / 4 CPU cycles) * ( 10 milliseconds )
#define NUM_TICKS (CPU_FREQ/MS_PER_S/TICKS_PER_CYCLE*NUM_MS_TO_DELAY)
#define DELAY_10MS for(unsigned int ___iii=0; ___iii<NUM_TICKS;___iii++){;}

#define DEFAULT_LED_VALUE 1

static int ms_tick;
static int release;

void toggle_red_led( void );
void toggle_green_led( void );
void toggle_yellow_led( void );

int main()
{
    int timer_dbc;

    // Clear interrupts right away
    cli();

    ms_tick = 0;
    release = 0;

    // Set up IO
    

    // Set up timers
    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescaler ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 1 ms (1000Hz) and need to find values of prescaler and timer_period (TOP, aka OCRnA)
    **
    ** 1000 = (20E6 / 1) * (1 / prescaler) * (1 / timer_period)
    ** prescaler = 20E6 / (1000 * timer_period)
    **
    ** prescaler = 256
    ** timer_period = 78
    **
    ** freq_interrupt [actual] = 20M / 256 / 78 = 1001.60Hz
    */

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescaler of 256
    timer_1284p_set_COM( TIMER_1284P_0, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_0, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_0, TIMER_1284P_CS_PRESCALE_DIV256);

#define TIMER0_HZ 1000
#define TIMER0_PRESCALER 256
#define TIMER0_PERIOD (CPU_FREQ/TIMER0_HZ/TIMER0_PRESCALER)
    // Timer period of 78 (8-bit register)
    timer_1284p_set_OCR( TIMER_1284P_0, TIMER_1284P_A, TIMER0_PERIOD - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_0, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_OVERFLOW );




    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescaler ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 100 ms (10Hz) and need to find values of prescaler and timer_period (TOP, aka OCRnA)
    **
    ** 10 = (20E6 / 1) * (1 / prescaler) * (1 / timer_period)
    ** prescaler = 20E6 / (10 * timer_period)
    **
    ** prescaler = 64
    ** timer_period = 31250
    **
    ** freq_interrupt [actual] = 20M / 256 / 7812 = 10.00064Hz
    */

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescaler of 256
    timer_1284p_set_COM( TIMER_1284P_3, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_3, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_3, TIMER_1284P_CS_PRESCALE_DIV64);

#define TIMER3_HZ 10
#define TIMER3_PRESCALER 64
#define TIMER3_PERIOD (CPU_FREQ/TIMER3_HZ/TIMER3_PRESCALER)
    // Timer period of 78 (8-bit register)
    timer_1284p_set_OCR( TIMER_1284P_3, TIMER_1284P_A, TIMER3_PERIOD - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_3, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_3, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_3, TIMER_1284P_IE_OVERFLOW );


    clear();

    // Set locals before enabling interrupts
    timer_dbc = 0;
    toggle_red_led();
    toggle_green_led();

    //Global interrupt enable
    sei();

    while( 1 )
    {
        lcd_goto_xy(0, 0);
        //print_long(TCNT0);
        
        for ( int i = 0; i < 50; i++ )
        {
            DELAY_10MS;
        }
        toggle_green_led();
        
        if ( release == 1 )
        {
            //Toggle red
            toggle_red_led();
            release = 0;
        }
	}
}

ISR(TIMER0_COMPA_vect)
{
    char cSREG;
    
    cSREG = SREG;
    
    ms_tick++;
    if ( ms_tick == 500 )
    {
        ms_tick = 0;
        release = 1;
    }
    
    SREG = cSREG;
}

ISR(TIMER3_COMPA_vect)
{
    char cSREG;
    static int task3_tick = 0;

    cSREG = SREG;

    task3_tick++;
    if ( task3_tick == 1 )
    {
        toggle_yellow_led();
        task3_tick = 0;
    }

    SREG = cSREG;
}


void toggle_red_led( void )
{
    static int red_LED_value = DEFAULT_LED_VALUE;
    red_led(red_LED_value);
    red_LED_value ^= 0x1;
}

void toggle_green_led( void )
{
    static int green_LED_value = DEFAULT_LED_VALUE;
    green_led(green_LED_value);
    green_LED_value ^= 0x1;
}

void toggle_yellow_led( void )
{
    static int yellow_LED_value = DEFAULT_LED_VALUE;
    set_digital_output(IO_A0, yellow_LED_value);  // PC1 is low, so drive PD1 low.
    yellow_LED_value ^= 0x1;
}
