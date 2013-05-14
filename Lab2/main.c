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

#define BUFFER_SIZE 64
#define LOOP_DELAY_MS 9
#define MAX_INT_OUTPUT 100
#define USB_BAUD_RATE 256000

// Position
#define DEG_PER_COUNT (360.0f / 64.0f)
#define POSITION_ERROR_MIN 1
#define POSITION_ERROR_MAX 64

// Velocity
#define V_ITER_THRESH 15

// Motor
#define MOTOR_SPEED_MIN                 25
#define MOTOR_SPEED_MAX                 175

// Encoder pin mapping
#define PIN_ENCODER_1A                  IO_A2
#define PIN_ENCODER_1B                  IO_A3
#define PIN_ENCODER_2A                  IO_A0
#define PIN_ENCODER_2B                  IO_A1


static int send_outputs;
static float Pr_f, Kp_f, Kd_f;

int main()
{
    int Pe_int, Pm_int, Pr_int, Vm_int, T_int;
    char buffer[BUFFER_SIZE];

    unsigned int v_iter = 0;
    unsigned int v_iter_last_pos = 0;
    
    unsigned int speed, direction;

    Pe_int = Pm_int = Pr_int = Vm_int = T_int = 0;
    Pr_f = Kp_f = Kd_f = 0.0f;

    send_outputs = 1;

    clear();

	play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.

    serial_set_baud_rate(USB_COMM, USB_BAUD_RATE);
    init_menu();
    
    clear();

    // Initialize the encoders and specify the four input pins, first two are for motor 1, second two are for motor 2
    encoders_init( PIN_ENCODER_1A, PIN_ENCODER_1B, PIN_ENCODER_2A, PIN_ENCODER_2B );

    // Dummy values for the time being
    Pr_f = 0.0f;
    Kp_f = 4.3f;
    Kd_f = -4.85f;

	while(1)
	{
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

        // Torque
        float t1_f = Kp_f * Pe_int;
        float t2_f = Kd_f * Vm_int;
        T_int = (int)(t1_f - t2_f);

        if ( T_int < -MOTOR_SPEED_MAX )
            T_int = -MOTOR_SPEED_MAX;

        if ( T_int > MOTOR_SPEED_MAX )
            T_int = MOTOR_SPEED_MAX;

        set_motors( 0, T_int );

        snprintf( buffer, BUFFER_SIZE, "v,%d,%d,%d,%d,%d,%d,%d\r\n", (signed int)Pe_int, (signed int)Pr_int, (signed int)Pm_int, (signed int)Vm_int, (signed int)T_int, (signed int)(Kp_f*1000), (signed int)(Kd_f*1000) );

        if ( send_outputs == 1 )
        {
            print_usb( buffer );
        }

        serial_check();
        check_for_new_bytes_received();

        delay_ms( LOOP_DELAY_MS );
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
