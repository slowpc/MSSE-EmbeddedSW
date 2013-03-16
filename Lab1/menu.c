#include "menu.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

void clr_red_toggle_counter( void );
void clr_green_toggle_counter( void );
void clr_yellow_toggle_counter( void );
int get_red_toggle_counter( void );
int get_green_toggle_counter( void );
int get_yellow_toggle_counter( void );
void set_red_period( int new_period );
void set_green_period( int new_period );
void set_yellow_period( int new_period );

//#define ECHO2LCD

// local "global" data structures
char receive_buffer[32];
unsigned char receive_buffer_position;
char send_buffer[32];

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

//------------------------------------------------------------------------------------------
// Initialize serial communication through USB and print menu options
// This immediately readies the board for serial comm
void init_menu() {
	
	//char printBuffer[32];
	
	// Set the baud rate to 9600 bits per second.  Each byte takes ten bit
	// times, so you can get at most 960 bytes per second at this speed.
	serial_set_baud_rate(USB_COMM, 9600);

	// Start receiving bytes in the ring buffer.
	serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));

	//memcpy_P( send_buffer, PSTR("USB Serial Initialized\r\n"), 24 );
	//snprintf( printBuffer, 24, "USB Serial Initialized\r\n");
	//print_usb( printBuffer, 24 );
	print_usb( "\r\n\nUSB Serial Initialized\r\n" );

	//memcpy_P( send_buffer, MENU, MENU_LENGTH );
	print_usb( MENU );
}

//------------------------------------------------------------------------------------------
// process_received_byte: Parses a menu command (series of keystrokes) that 
// has been received on USB_COMM and processes it accordingly.
// The menu command is buffered in check_for_new_bytes_received (which calls this function).
void process_received_string(const char* buffer)
{
	// Used to pass to USB_COMM for serial communication
	char tempBuffer[32];
	
	// parse and echo back to serial comm window (and optionally the LCD)
	char color;
	char op_char;
	int value;
	int parsed;

    cli();

	parsed = sscanf(buffer, "%c %c %d", &op_char, &color, &value);
#ifdef ECHO2LCD
	lcd_goto_xy(0,0);
	printf("Got %c %c %d\n", op_char, color, value);
#endif
	sprintf( tempBuffer, "Op:%c C:%c V:%d\r\n", op_char, color, value );
	print_usb( tempBuffer );
	
	// convert color to upper and check if valid
	color -= 32*(color>='a' && color<='z');
	switch (color) {
		case 'R':
		case 'G':
		case 'Y': 
		case 'A': break;
		default:
			print_usb( "Bad Color. Try {RGYA}\r\n" );
			print_usb( MENU );
			return;
	}

	// Check valid command and implement
	switch (op_char) {
		// change toggle frequency for <color> LED
		case 'T':
		case 't':
		    switch(color) {
    		    case 'R':
    		        set_red_period( value );
    		        sprintf( tempBuffer, "R freq: %d\r\n", value );
    		        print_usb( tempBuffer );
    		        break;
    		    case 'G':
    		        set_green_period( value );
    		        sprintf( tempBuffer, "G freq: %d\r\n", value );
    		        print_usb( tempBuffer );
    		        break;
    		    case 'Y':
    		        set_yellow_period( value );
    		        sprintf( tempBuffer, "Y freq: %d\r\n", value );
    		        print_usb( tempBuffer );
    		        break;
    		    case 'A':
    		        set_red_period( value );
    		        set_green_period( value );
    		        set_yellow_period( value );
    		        sprintf( tempBuffer, "Freq R:%d G:%d Y:%d\r\n", value, value, value );
    		        print_usb( tempBuffer );
    		        break;
    		    default: print_usb("Default in t(color). How?\r\n" );
            }
		    break;
		// print counter for <color> LED 
		case 'P':
		case 'p':
			switch(color) {
				case 'R': 
					sprintf( tempBuffer, "R toggles: %d\r\n", get_red_toggle_counter() );
					print_usb( tempBuffer ); 
					break;
				case 'G': 
					sprintf( tempBuffer, "G toggles: %d\r\n", get_green_toggle_counter() );
					print_usb( tempBuffer ); 
					break;
				case 'Y': 
					sprintf( tempBuffer, "Y toggles: %d\r\n", get_yellow_toggle_counter() );
					print_usb( tempBuffer ); 
					break;
				case 'A': 
					sprintf( tempBuffer, "Toggles R:%d G:%d Y:%d\r\n", get_red_toggle_counter(), get_green_toggle_counter(), get_yellow_toggle_counter() );
					print_usb( tempBuffer ); 
					break;
				default: print_usb("Default in p(color). How?\r\n" );
			}
			break;

		// zero counter for <color> LED 
		case 'Z':
		case 'z':
			switch(color) {
				case 'R':
                    clr_red_toggle_counter();
                    sprintf( tempBuffer, "Zero R\r\n" );
                    print_usb( tempBuffer );
                    break;
				case 'G':
                    clr_green_toggle_counter();
                    sprintf( tempBuffer, "Zero G\r\n" );
                    print_usb( tempBuffer );
                    break;
				case 'Y':
				    sprintf( tempBuffer, "Zero Y\r\n" );
				    print_usb( tempBuffer );
                    clr_yellow_toggle_counter();
                    break;
				case 'A':
                    clr_red_toggle_counter();
                    clr_green_toggle_counter();
                    clr_yellow_toggle_counter();
                    sprintf( tempBuffer, "Zero All\r\n" );
                    print_usb( tempBuffer );
                    break;
				default: print_usb("Default in z(color). How?\r\n" );
			}
			break;
		default:
			print_usb( "Command does not compute.\r\n" );
		} // end switch(op_char) 
		
	print_usb( MENU );

    sei();

} //end menu()

//---------------------------------------------------------------------------------------
// If there are received bytes to process, this function loops through the receive_buffer
// accumulating new bytes (keystrokes) in another buffer for processing.
void check_for_new_bytes_received()
{
	/* 
	The receive_buffer is a ring buffer. The call to serial_check() (you should call prior to this function) fills the buffer.
	serial_get_received_bytes is an array index that marks where in the buffer the most current received character resides. 
	receive_buffer_position is an array index that marks where in the buffer the most current PROCESSED character resides. 
	Both of these are incremented % (size-of-buffer) to move through the buffer, and once the end is reached, to start back at the beginning.
	This process and data structures are from the Pololu library. See examples/serial2/test.c and src/OrangutanSerial/ *
	
	A carriage return from your comm window initiates the transfer of your keystrokes.
	All key strokes prior to the carriage return will be processed with a single call to this function (with multiple passes through this loop).
	On the next function call, the carriage return is processes with a single pass through the loop.
	The menuBuffer is used to hold all keystrokes prior to the carriage return. The "received" variable, which indexes menuBuffer, is reset to 0
	after each carriage return.
	*/ 
	char menuBuffer[32];
	static int received = 0;
    int evaluate = 0;
	
	// while there are unprocessed keystrokes in the receive_buffer, grab them and buffer
	// them into the menuBuffer
	while(serial_get_received_bytes(USB_COMM) != receive_buffer_position)
	{
		// place in a buffer for processing
		menuBuffer[received] = receive_buffer[receive_buffer_position];

print_usb_char( menuBuffer[received] );

#ifdef ECHO2LCD
lcd_goto_xy(0,0);
print("RX: (");
print_long(menuBuffer[received]);
print_character(')');
for (int i=0; i<received; i++)
{
    print_character(menuBuffer[i]);
}
#endif

        if ( menuBuffer[received] == '\r' )
        {
            print_usb( "\n" );
            evaluate = 1;
        }

		++received;
		
		// Increment receive_buffer_position, but wrap around when it gets to
		// the end of the buffer. 
		if ( receive_buffer_position == sizeof(receive_buffer) - 1 )
		{
			receive_buffer_position = 0;
		}			
		else
		{
			receive_buffer_position++;
		}
	}
#ifdef ECHO2LCD
		lcd_goto_xy(0,1);
		print("RX: (");
		print_long(received);
		print_character(')');
		for (int i=0; i<received; i++)
		{
			print_character(menuBuffer[i]);
		}
#endif
	// If there were keystrokes processed, check if a menu command
	if ( evaluate ) {/*
		// if only 1 received, it was MOST LIKELY a carriage return. 
		// Even if it was a single keystroke, it is not a menu command, so ignore it.
		if ( 1 == received ) {
			received = 0;
			return;
		}*/
		// Process buffer: terminate string, process, reset index to beginning of array to receive another command
		menuBuffer[received] = '\0';
#ifdef ECHO2LCD
		lcd_goto_xy(0,1);
		print("RX: (");
		print_long(received);
		print_character(')');
		for (int i=0; i<received; i++)
		{
			print_character(menuBuffer[i]);
		}
#endif
		process_received_string(menuBuffer);
		received = 0;
	}
}
	
//-------------------------------------------------------------------------------------------
// wait_for_sending_to_finish:  Waits for the bytes in the send buffer to
// finish transmitting on USB_COMM.  We must call this before modifying
// send_buffer or trying to send more bytes, because otherwise we could
// corrupt an existing transmission.
void wait_for_sending_to_finish()
{
	while(!serial_send_buffer_empty(USB_COMM))
		serial_check();		// USB_COMM port is always in SERIAL_CHECK mode
}

