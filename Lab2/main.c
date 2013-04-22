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
#define LOOP_DELAY_MS 50
#define MAX_INT_OUTPUT 100
#define USB_BAUD_RATE 256000

// Position
#define DEG_PER_COUNT (360.0f / 64.0f)
#define POSITION_ERROR_MIN 10
#define POSITION_ERROR_MAX 45

// Velocity
#define V_ITER_THRESH 15

// Motor
#define MOTOR_SPEED_MIN                 20
#define MOTOR_SPEED_MAX                 100

// Encoder pin mapping
#define PIN_ENCODER_1A                  IO_A2
#define PIN_ENCODER_1B                  IO_A3
#define PIN_ENCODER_2A                  IO_A0
#define PIN_ENCODER_2B                  IO_A1


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
    signed int Pm_count;
    float Pe, Pr, Pm, Vm, T, Kp, Kd;
    char buffer[BUFFER_SIZE];

    unsigned int v_iter = 0;
    unsigned int v_iter_last_pos = 0;
    unsigned int v_initialized = 0;
    
    unsigned int speed, direction;

    Pe = Pr = Pm = Vm = T=  Kp = Kd = 0;

    clear();

	play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.

    serial_set_baud_rate(USB_COMM, USB_BAUD_RATE);
    init_menu();

    // Initialize the encoders and specify the four input pins, first two are for motor 1, second two are for motor 2
    encoders_init( PIN_ENCODER_1A, PIN_ENCODER_1B, PIN_ENCODER_2A, PIN_ENCODER_2B );

    // Dummy values for the time being
    Pr = 360;
    Kp = 0.05f;
    Kd = -.05f;

	while(1)
	{
        // Calc current position
        Pm_count  = encoders_get_counts_m2();

        Pm = Pm_count * DEG_PER_COUNT;

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

        if ( Pe < -POSITION_ERROR_MAX )
            Pe = -POSITION_ERROR_MAX;


        // Torque
        T = Kp * Pe - Kd * Vm;

        


        if ( T < -MOTOR_SPEED_MAX )
            T = -MOTOR_SPEED_MAX;

        if ( T > MOTOR_SPEED_MAX )
            T = MOTOR_SPEED_MAX;

        if ( T < 0 )
        {
            speed = -T;
            direction = 0;
        }
        else
        {
            speed = T;
            direction = 1;
        }

        // Force the motor to the minimum amount if it hasn't reached the goal yet
        if (abs(Pe) > POSITION_ERROR_MIN && speed < MOTOR_SPEED_MIN )
            speed = MOTOR_SPEED_MIN;

        // TODO: Replace with setting registers
        signed int T_temp;
        T_temp = ( direction == 1 ) ? speed : -speed;
        set_motors( 0, T_temp );

        snprintf( buffer, BUFFER_SIZE, "v,%d,%d,%d,%d,%d,%d,%d\r\n", (signed int)Pe, (signed int)Pr, (signed int)Pm, (signed int)Vm, (signed int)T_temp, (signed int)Kp, (signed int)Kd );

        print_usb( buffer );

//        serial_check();
//        check_for_new_bytes_received();

        delay_ms( LOOP_DELAY_MS );
	}
}
