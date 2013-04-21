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

#define BUFFER_SIZE 32
#define LOOP_DELAY 50
#define LOOP_DELAY_ACTUAL (LOOP_DELAY / 2)
#define MAX_INT_OUTPUT 100
#define USB_BAUD_RATE 256000


void print_usb_char( char buffer );
void print_usb( char *buffer );
void wait_for_sending_to_finish( void );

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
}

int main()
{
    unsigned int i, j, k;
    char buffer[BUFFER_SIZE];

    i = j = k = 0;

    clear();

print_long(1);
	play_from_program_space(PSTR(">g32>>c32"));  // Play welcoming notes.


print_long(2);
    serial_set_baud_rate(USB_COMM, USB_BAUD_RATE);

print_long(3);
	while(1)
	{
        /*
		// Get battery voltage (in mV) from the auxiliary processor
		// and print it on the the LCD.
		clear();
		print_long(read_battery_millivolts_svp());

		red_led(1);     // Turn on the red LED.
		delay_ms(200);  // Wait for 200 ms.

		red_led(0);     // Turn off the red LED.
		delay_ms(200);  // Wait for 200 ms.
        */

print_long(4);
        snprintf( buffer, BUFFER_SIZE, "%d,%d,%d\r\n", i, j, k );

print_long(5);
        print_usb( buffer );

print_long(6);
        clear();
        print_long(read_battery_millivolts_svp());

        red_led(1);     // Turn on the red LED.
        delay_ms( LOOP_DELAY_ACTUAL );

        red_led(0);     // Turn off the red LED.
        delay_ms( LOOP_DELAY_ACTUAL );

        if ( i % 1 == 0 )
        {
            i++;
            if ( i >= MAX_INT_OUTPUT )
                i = 0;
        }

        if ( i % 2 == 0 )
        {
            j++;
            if ( j >= MAX_INT_OUTPUT )
                j = 0;
        }

        if ( i % 3 == 0 )
        {
            k++;
            if ( k >= MAX_INT_OUTPUT )
                k= 0;
        }
	}
}
