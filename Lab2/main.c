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

//#include "menu.h"

#define BUFFER_SIZE 64
#define LOOP_DELAY_US 10000
#define MAX_INT_OUTPUT 100
#define USB_BAUD_RATE 256000

// Position
#define POSITION_ERROR_MIN 3
#define POSITION_ERROR_MAX 30

// Velocity
#define V_ITER_THRESH 15

// Motor
#define MOTOR_SPEED_MIN                 16
#define MOTOR_SPEED_MAX                 50

// Encoder pin mapping
#define PIN_ENCODER_1A                  IO_A2
#define PIN_ENCODER_1B                  IO_A3
#define PIN_ENCODER_2A                  IO_A0
#define PIN_ENCODER_2B                  IO_A1

// Encoder
#define ENCODER_COUNT_HYS_RANGE         8
#define ENCODER_COUNT_PER_REV           32
#define ENCODER_NUM_REVS                2
#define ENCODER_ABS_MAX                 ( ENCODER_COUNT_PER_REV * ENCODER_NUM_REVS )
#define ENCODER_MIN                     -ENCODER_ABS_MAX
#define ENCODER_MAX                     ENCODER_ABS_MAX
#define ENCODER_START                   -ENCODER_ABS_MAX


void print_usb_char( char buffer );
void print_usb( char *buffer );
void wait_for_sending_to_finish( void );
/*
// A generic function for whenever you want to print to your serial comm window.
// Provide a string and the length of that string. My serial comm likes "\r\n" at
// the end of each string (be sure to include in length) for proper linefeed.
void print_usb_char( char buffer ) {
    char local_buf[2];
    local_buf[0] = buffer;
    local_buf[1] = NULL;
    print_usb( local_buf );
}

void print_usb( char *buffer )
{
    int length;
    length = strlen( buffer );
    serial_send( USB_COMM, buffer, length );
    wait_for_sending_to_finish();
}

//-------------------------------------------------------------------------------------------
// wait_for_sending_to_finish:  Waits for the bytes in the send buffer to
// finish transmitting on USB_COMM.  We must call this before modifying
// send_buffer or trying to send more bytes, because otherwise we could
// corrupt an existing transmission.
void wait_for_sending_to_finish( void )
{
	while(!serial_send_buffer_empty(USB_COMM))
		serial_check();		// USB_COMM port is always in SERIAL_CHECK mode
}*/

int main()
{
    signed int Pe, Pr, Pm, Vm, T;//, Kp, Kd;
    float Kp, Kd;
    unsigned int Pr_status, Pm_status, Vm_status, T_status, Kp_status, Kd_status;
    char buffer[BUFFER_SIZE];
    
    signed int Kp_int;
    signed int Kd_int;

    unsigned int v_iter = 0;
    unsigned int v_iter_last_pos = 0;
    unsigned int v_initialized = 0;

    Pe = Pr = Pm = Vm = T=  Kp = Kd = 0;

    clear();

	play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.

    serial_set_baud_rate(USB_COMM, USB_BAUD_RATE);
    init_menu();

    // Initialize the encoders and specify the four input pins, first two are for motor 1, second two are for motor 2
    encoders_init( PIN_ENCODER_1A, PIN_ENCODER_1B, PIN_ENCODER_2A, PIN_ENCODER_2B );

    Pr = 64;

	while(1)
	{
        // Calc current position
        Pm        = encoders_get_counts_m2();
        Pm_status = encoders_check_error_m2();

        // Calc velocity
        if ( v_iter++ > V_ITER_THRESH )
        {
            v_iter = 0;
            Vm = Pm - v_iter_last_pos;
            v_iter_last_pos = Pm;
        }

        // Calculate the position error
        Pe = Pr - Pm;

        if ( Pe > POSITION_ERROR_MAX )
            Pe = POSITION_ERROR_MAX;


        // Torque
        T = (int) (Kp * 1.0f * Pe - Kd * 1.0f * Vm);

/*
        if ( Pm < MOTOR_SPEED_MIN )
        {
            direction = 1;
            speed = MOTOR_SPEED_MIN;
        }
        else
        {
            speed = Pm;
            
            if ( direction == 1 && Pm > 100)
            {
                direction = 0;
                Pr = 32;
            }
            else if ( direction == 0 && Pm < 32 )
            {
                direction = 1;
                Pr = 100;
            }
        }*/

        if (Pe > POSITION_ERROR_MIN && T < MOTOR_SPEED_MIN )
            T = MOTOR_SPEED_MIN;

        if ( T < -MOTOR_SPEED_MAX )
            T = -MOTOR_SPEED_MAX;

        if ( T > MOTOR_SPEED_MAX )
            T = MOTOR_SPEED_MAX;

        set_motors( 0, T );

        // Dummy values for the time being
        Kp = 10.0f;
        Kd = -15.0f;
        Kp_int = (signed int) Kp;
        Kd_int = (signed int) Kd;
        snprintf( buffer, BUFFER_SIZE, "%d,%d,%d,%d,%d,%d,%d\r\n", Pe, Pr, Pm, Vm, T, Kp_int, Kd_int );

        print_usb( buffer );

        delay_us( LOOP_DELAY_US );
	}
}
