/* Lab2 - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 4/12/2013 3:41:41 PM
 *  Author: Kyle
 */

#include <pololu/orangutan.h>

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "menu.h"
#include "timer_1284p.h"

// Timer frequencies
#define TIMER0_HZ 1000

// Timer prescalers
#define TIMER0_PRESCALER 256

// CPU Definitions
#define CPU_FREQ 20000000

#define BUFFER_SIZE 64
#define LOOP_DELAY_MS 9
#define MAX_INT_OUTPUT 100
#define USB_BAUD_RATE 256000

// Position
#define DEG_PER_COUNT (360.0f / 64.0f)
#define POSITION_ERROR_DEG_MAX 360.0f
#define POSITION_ERROR_COUNT_MAX (POSITION_ERROR_DEG_MAX / DEG_PER_COUNT)

// Velocity
#define V_ITER_THRESH 100

// Motor
#define MOTOR_SPEED_MIN                 25
#define MOTOR_SPEED_MAX                 175

// Encoder pin mapping
#define PIN_ENCODER_1A                  IO_A2
#define PIN_ENCODER_1B                  IO_A3
#define PIN_ENCODER_2A                  IO_A0
#define PIN_ENCODER_2B                  IO_A1

static void calculate();
static void service_serial();

void set_timer0( void );

static int send_outputs;
static float Pr_f, Kp_f, Kd_f;
static int Pe_int, Pm_int, Pr_int, Vm_int, T_int;

int main()
{
    Pe_int = Pm_int = Pr_int = Vm_int = T_int = 0;

    Pr_f = 0.0f, Kp_f = 4.3f, Kd_f = -4.85f; // Dummy values until new ones are set at runtime

    send_outputs = 1; // Default to send outputs

    clear();

    play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.

    serial_set_baud_rate(USB_COMM, USB_BAUD_RATE);
    init_menu();

    clear();

    // Initialize the encoders and specify the four input pins, first two are for motor 1, second two are for motor 2
    encoders_init( PIN_ENCODER_1A, PIN_ENCODER_1B, PIN_ENCODER_2A, PIN_ENCODER_2B );

    set_timer0();

    // Calculate first values
    calculate();

    // Global interrupt enable
    sei();

    while(1)
    {
        service_serial();
        delay_ms( LOOP_DELAY_MS );
    }
}

static void calculate()
{
    static unsigned int v_iter = 0;
    static unsigned int v_iter_last_pos = 0;

    // Calc current position
    Pm_int  = encoders_get_counts_m2();

    // Calc velocity
    if ( v_iter++ > V_ITER_THRESH )
    {
        v_iter = 0;
        Vm_int = Pm_int - v_iter_last_pos;
        v_iter_last_pos = Pm_int;
    }

    // Calculate the position error
    Pr_int = (int)(Pr_f  / DEG_PER_COUNT);
    Pe_int = Pr_int - Pm_int;

    if ( Pe_int > POSITION_ERROR_COUNT_MAX )
    {
        Pe_int = POSITION_ERROR_COUNT_MAX;
    }

    if ( Pe_int < -POSITION_ERROR_COUNT_MAX )
    {
        Pe_int = -POSITION_ERROR_COUNT_MAX;
    }

    // Torque
    float t1_f = Kp_f * Pe_int;
    float t2_f = Kd_f * Vm_int;
    T_int = (int)(t1_f - t2_f);

    if ( T_int < -MOTOR_SPEED_MAX )
    {
        T_int = -MOTOR_SPEED_MAX;
    }

    if ( T_int > MOTOR_SPEED_MAX )
    {
        T_int = MOTOR_SPEED_MAX;
    }

    set_motors( 0, T_int );
}

static void service_serial()
{
    static char buffer[BUFFER_SIZE];

    // check for new serial input command
    serial_check();
    check_for_new_bytes_received();

    snprintf( buffer, BUFFER_SIZE, "v,%d,%d,%d,%d,%d,%d,%d\r\n", (signed int)Pe_int, (signed int)Pr_int, (signed int)Pm_int, (signed int)Vm_int, (signed int)T_int, (signed int)(Kp_f*1000), (signed int)(Kd_f*1000) );

    if ( send_outputs == 1 )
    {
        print_usb( buffer );
    }
}

void set_logging( int new_value )
{
    send_outputs = new_value;
}

void set_Pr( float new_ref )
{
    Pr_f += new_ref;
}

void set_Kp( float new_Kp )
{
    Kp_f = new_Kp;
}

void set_Kd( float new_Kd )
{
    Kd_f = new_Kd;
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
    timer_1284p_set_COM( TIMER_1284P_0, TIMER_1284P_A, TIMER_1284P_COM_NO_COMPARE);
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

ISR(TIMER0_COMPA_vect)
{
    char cSREG;

    cSREG = SREG;

    calculate();

    SREG = cSREG;
}
