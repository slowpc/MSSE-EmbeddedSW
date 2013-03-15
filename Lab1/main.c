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

// Includes
#include <pololu/orangutan.h>
#include <avr/io.h>         //gives us names for registers
#include <avr/interrupt.h>
#include "timer_1284p.h"
#include "usart.h"

#define PRINT_COUNTERS 0

// Timer frequencies
#define TIMER0_HZ 1000
#define BUSY_WAIT_HZ 100
#define TIMER3_HZ 10

// Timer prescalers
#define TIMER0_PRESCALER 256
#define TIMER1_PRESCALER 1024
#define TIMER3_PRESCALER 64

// Conversions
#define MS_PER_S 1000

// CPU Definitions
#define CPU_FREQ 20000000

// Busy waiting
#define NUM_MS_TO_WAIT ( MS_PER_S / BUSY_WAIT_HZ )
#define TICKS_PER_CYCLE 4 // Empty for loop with optimization of -1 has 2 instructions that each take 2 cycles until the last iteration
// ( 20M CPU cycles / 1 second ) * ( 1 second / 1000 milliseconds ) * ( 1 loop / 4 CPU cycles) * ( 10 milliseconds )
#define NUM_TICKS (CPU_FREQ/MS_PER_S/TICKS_PER_CYCLE*NUM_MS_TO_WAIT)
#define BUSY_WAIT for(unsigned int ___iii=0; ___iii<NUM_TICKS;___iii++){;}

// Initial periods
#define DEFAULT_PERIOD_MS_RED       500
#define DEFAULT_PERIOD_MS_GREEN     500
#define DEFAULT_PERIOD_MS_YELLOW    100

// Initial LED
#define DEFAULT_LED_VALUE 1

// LED pin mapping
#define LED_RED             IO_A2
#define LED_YELLOW          IO_A0
#define LED_GREEN           IO_D5 // This pin isn't used because HW sets it
#define LED_PORT_RED        DDRA
#define LED_PORT_YELLOW     DDRA
#define LED_PORT_GREEN      DDRD
#define LED_PORT_RED_BIT    DDD2
#define LED_PORT_YELLOW_BIT DDD0
#define LED_PORT_GREEN_BIT  DDD5
//#define LED_GREEN       IO_D5 // This is LED is actually driven from PWM on this pin.

static int release;
static int use_busy_wait;

static int tick_threshold_red;
static int tick_threshold_red_busy;
static int tick_threshold_green;
static int tick_threshold_yellow;
static float timer1_hz;

static int toggle_counter_ms_red;
static int toggle_counter_ms_green;
static int toggle_counter_ms_yellow;

void task_red_led( void );
void task_green_led( void );
void task_yellow_led( void );

void clr_red_toggle_counter( void );
void clr_green_toggle_counter( void );
void clr_yellow_toggle_counter( void );

void set_red_period( int );
void set_green_period( int );
void set_yellow_period( int );

void set_timer0( void );
void set_timer1( void );
void set_timer3( void );

int main()
{

    // Clear interrupts right away
    cli();

    release = 0;
    use_busy_wait = 0;
    tick_threshold_red = 0;
    tick_threshold_red_busy = 0;
    tick_threshold_green = 0;
    tick_threshold_yellow = 0;
    toggle_counter_ms_red = 0;
    toggle_counter_ms_green = 0;
    toggle_counter_ms_yellow = 0;
    timer1_hz = 0.0f;
    clear();

    // Set up IO
    LED_PORT_RED    |= ( 1 << LED_PORT_RED_BIT );
    LED_PORT_GREEN  |= ( 1 << LED_PORT_GREEN_BIT );
    LED_PORT_YELLOW |= ( 1 << LED_PORT_YELLOW_BIT );

    // Set up USART
    init_USART1();

    // Set up timers
    set_red_period( DEFAULT_PERIOD_MS_RED );
    set_green_period( DEFAULT_PERIOD_MS_GREEN ); // This needs to be called before setting the timers
    set_yellow_period( DEFAULT_PERIOD_MS_YELLOW );
    set_timer0();
    set_timer1();
    set_timer3();

    // Set locals before enabling interrupts
    clr_red_toggle_counter();
    clr_green_toggle_counter();
    clr_yellow_toggle_counter();
    task_red_led();
    task_green_led();
    task_yellow_led();

    // Global interrupt enable
    sei();

    while( 1 )
    {
        if ( use_busy_wait )
        {
            for ( int i = 0; i < tick_threshold_red_busy; i++ )
            {
                BUSY_WAIT;
            }
            //Toggle red
            task_red_led();
        }
        else
        {
            if ( release == 1 )
            {
                //Toggle red
                task_red_led();
                release = 0;
            }
        }

#if PRINT_COUNTERS
        cli();
        lcd_goto_xy(0,0);
        sei();
        cli();
        print("R:");
        sei();
        cli();
        print_long(toggle_counter_ms_red);
        sei();
        cli();
        lcd_goto_xy(8,0);
        sei();
        cli();
        print("G:");
        sei();
        cli();
        print_long(toggle_counter_ms_green);
        sei();
        cli();
        lcd_goto_xy(0,1);
        sei();
        cli();
        print("Y:");
        sei();
        cli();
        print_long(toggle_counter_ms_yellow);
        sei();
#endif
    }
}

ISR(TIMER0_COMPA_vect)
{
    char cSREG;
    static int task0_tick = 0;

    cSREG = SREG;

    task0_tick++;

    // If task is enabled and reached its 'release' time
    if ( ( tick_threshold_red != 0 ) && ( task0_tick >= tick_threshold_red ) )
    {
        task0_tick = 0;
        release = 1;
    }

    SREG = cSREG;
}

ISR(TIMER1_COMPA_vect)
{
    char cSREG;

    cSREG = SREG;

    // Check if task is enabled
    if ( timer1_hz )
    {
        task_green_led();
    }

    SREG = cSREG;
}

ISR(TIMER3_COMPA_vect)
{
    char cSREG;
    static int task3_tick = 0;

    cSREG = SREG;

    task3_tick++;

    // If task is enabled and reached its 'release' time
    if ( ( tick_threshold_yellow != 0 ) && ( task3_tick >= tick_threshold_yellow ) )
    {
        task3_tick = 0;
        task_yellow_led();
    }

    SREG = cSREG;
}

void task_red_led( void )
{
    static int red_LED_value = DEFAULT_LED_VALUE;
    set_digital_output(LED_RED, red_LED_value);
    red_led(red_LED_value);
    red_LED_value ^= 0x1;
    toggle_counter_ms_red++;

    usart_check();
}

void task_green_led( void )
{
    // This function should only be used to increment the toggle counter, but until the LED is connected to OC1A, use the green_led function

    static int green_LED_value = DEFAULT_LED_VALUE;
    green_led(green_LED_value);
    green_LED_value ^= 0x1;
    toggle_counter_ms_green++;
}

void task_yellow_led( void )
{
    static int yellow_LED_value = DEFAULT_LED_VALUE;
    set_digital_output(LED_YELLOW, yellow_LED_value);
    yellow_LED_value ^= 0x1;
    toggle_counter_ms_yellow++;
}

void clr_red_toggle_counter( void )
{
    toggle_counter_ms_red = 0;
}

void clr_green_toggle_counter( void )
{
    toggle_counter_ms_green = 0;
}

void clr_yellow_toggle_counter( void )
{
    toggle_counter_ms_yellow = 0;
}

void set_red_period( int new_period )
{
    tick_threshold_red      = (int) ((float)new_period / (float)MS_PER_S * (float)TIMER0_HZ);
    tick_threshold_red_busy = (int) ((float)new_period / (float)MS_PER_S * (float)BUSY_WAIT_HZ);
}

void set_green_period( int new_period )
{
    int timer1_counter;

    cli();

    if ( new_period )
    {
        timer1_hz = (float)MS_PER_S / (float)new_period;
        timer1_counter = (int) ((float)CPU_FREQ/timer1_hz/(float)TIMER1_PRESCALER);
    }
    else
    {
        timer1_hz = 0;
        timer1_counter = 0xFFFF;
    }

    timer_1284p_set_OCR( TIMER_1284P_1, TIMER_1284P_A, timer1_counter - 1 );
}

void set_yellow_period( int new_period )
{
    tick_threshold_yellow = (int) ((float)new_period / (float)MS_PER_S * (float)TIMER3_HZ);
}

void set_timer0( void )
{
    cli();

    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescaler ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 1 ms (1000Hz) and need to find values of prescaler and timer_period (TOP, aka OCRnA)
    **
    ** hz = (20E6 / 1) * (1 / prescaler) * (1 / timer_period)
    ** timer_period = 20E6 / (hz * prescaler )
    ** prescaler    = 20E6 / (hz * timer_period )
    **
    ** prescaler = 256
    ** timer_period = 78
    **
    ** freq_interrupt [actual] = 20M / 256 / 78 = 1001.603Hz
    */

    timer_1284p_clr_counter( TIMER_1284P_0 );

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescaler of 256
    timer_1284p_set_COM( TIMER_1284P_0, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_0, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_0, TIMER_1284P_CS_PRESCALE_DIV256);

#define TIMER0_COUNTER ( (int) ((float)CPU_FREQ/(float)TIMER0_HZ/(float)TIMER0_PRESCALER) )
    // Timer period of 78 (8-bit register)
    timer_1284p_set_OCR( TIMER_1284P_0, TIMER_1284P_A, TIMER0_COUNTER - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_0, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_0, TIMER_1284P_IE_OVERFLOW );
}

void set_timer1( void )
{
    int timer1_counter;

    cli();

    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescaler ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 100 ms (10Hz) and need to find values of prescaler and timer_period (TOP, aka OCRnA)
    **
    ** hz = (20E6 / 1) * (1 / prescaler) * (1 / timer_period)
    ** timer_period = 20E6 / (hz * prescaler)
    ** prescaler    = 20E6 / (hz * timer_period)
    **
    ** prescaler = 64
    ** timer_period = 31250
    **
    ** freq_interrupt [actual] = 20M / 64 / 31250 = 10Hz
    */

    timer_1284p_clr_counter( TIMER_1284P_1 );

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescaler of 64
    timer_1284p_set_COM( TIMER_1284P_1, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_1, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_1, TIMER_1284P_CS_PRESCALE_DIV1024);

    if ( timer1_hz )
    {
        timer1_counter = (int) ((float)CPU_FREQ/timer1_hz/(float)TIMER1_PRESCALER);
    }
    else
    {
        timer1_counter = 0xFFFF;
    }

    // Timer period of 78 (16-bit register)
    timer_1284p_set_OCR( TIMER_1284P_1, TIMER_1284P_A, timer1_counter - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_1, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_1, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_1, TIMER_1284P_IE_OVERFLOW );
}

void set_timer3( void )
{
    cli();

    /*
    ** freq_interrupt = (CPU_freq / 1 second) * (1 count / prescaler ticks) * (1 interrput / timer_period counts)
    **
    ** CPU_freq is 20MHz.
    ** We want the frequency of the interrupt to be 100 ms (10Hz) and need to find values of prescaler and timer_period (TOP, aka OCRnA)
    **
    ** hz = (20E6 / 1) * (1 / prescaler) * (1 / timer_period)
    ** timer_period = 20E6 / (hz * prescaler)
    ** prescaler    = 20E6 / (hz * timer_period)
    **
    ** prescaler = 64
    ** timer_period = 31250
    **
    ** freq_interrupt [actual] = 20M / 64 / 31250 = 10Hz
    */

    timer_1284p_clr_counter( TIMER_1284P_3 );

    //Compare Output mode to toggle for OC0A
    //Waveform generation mode for CTC (Clear Timer on Compare Match mode)
    //Prescaler of 64
    timer_1284p_set_COM( TIMER_1284P_3, TIMER_1284P_A, TIMER_1284P_COM_TOGGLE);
    timer_1284p_set_WGM( TIMER_1284P_3, TIMER_1284P_WGM_CTC );
    timer_1284p_set_CS( TIMER_1284P_3, TIMER_1284P_CS_PRESCALE_DIV64);

#define TIMER3_COUNTER ( (int) ((float)CPU_FREQ/(float)TIMER3_HZ/(float)TIMER3_PRESCALER) )
    // Timer period of 78 (16-bit register)
    timer_1284p_set_OCR( TIMER_1284P_3, TIMER_1284P_A, TIMER3_COUNTER - 1 );

    // Disable interrupts for 0B, enable for 0A, and disable for 0 overflow
    timer_1284p_clr_IE( TIMER_1284P_3, TIMER_1284P_IE_B );
    timer_1284p_set_IE( TIMER_1284P_3, TIMER_1284P_IE_A );
    timer_1284p_clr_IE( TIMER_1284P_3, TIMER_1284P_IE_OVERFLOW );
}
