#include "usart.h"
#include "cbuf.h"

#include <avr/interrupt.h>
#include <pololu/orangutan.h>



// receive_buffer: A ring buffer that we will use to receive bytes on USB_COMM.
// The OrangutanSerial library will put received bytes in to
// the buffer starting at the beginning (receiveBuffer[0]).
// After the buffer has been filled, the library will automatically
// start over at the beginning.
char receive_buffer[32];

// receive_buffer_position: This variable will keep track of which bytes in the receive buffer
// we have already processed.  It is the offset (0-31) of the next byte
// in the buffer to process.
unsigned char receive_buffer_position = 0;

// send_buffer: A buffer for sending bytes on USB_COMM.
char send_buffer[32];

void check_for_new_bytes_received();



//our transmit and receive buffers
static uint8_t tx_buf[DEFAULT_BUF_SIZE];
static volatile cbuf_t  usart1_tx;

static uint8_t rx_buf[DEFAULT_BUF_SIZE];
static volatile cbuf_t  usart1_rx;

#define INPUT_BUFFER_SIZE 40

static uint8_t usart_input[INPUT_BUFFER_SIZE];

// This was externed, may need to change this or move to somewhere else
uint8_t G_flag;

//setup USART1 to receive/transmit
void init_USART1()
{
    // Set the baud rate to 9600 bits per second.  Each byte takes ten bit
    // times, so you can get at most 960 bytes per second at this speed.
    serial_set_baud_rate(USB_COMM, 9600);

    // Start receiving bytes in the ring buffer.
    serial_receive_ring(USB_COMM, receive_buffer, sizeof(receive_buffer));

/*
  //calculate the value for the UBRR register using the equation
  //given on page 175
  //  UBRR = ( osc / (16 * BAUD) ) - 1
  uint16_t ubrr_val = (uint16_t)((float)CLOCK_FREQ / (16.0f*(float)BAUD) ) - 1;

  //set up the baud speed
  UBRR1H = (ubrr_val >> 8) & 0xFF;
  UBRR1L = ubrr_val & 0xFF;

  //now we have to configure the USART
  //There are three registers we have to set; UCSR0(A,B,C)
  UCSR1A = 0;        //page 190 (status bits and double speed)
  UCSR1B =           //page 191
    ((1 << RXCIE1) | //enable interrupt on data receipt
     (1 << TXEN1)  | //enable transmitter
     (1 << RXEN1)    //enable receiver
     );              //done

  //this sets the serial port transmit parameters
  //we will be using 8n1, which is 8 data bits, no parity, 1 stop bit
  UCSR1C =            //page 192
    ((1 << UCSZ11) |  //these two specify 8 data bits
     (1 << UCSZ10) |
     (0 << USBS1)     // stop bits: 0 = 1-bit, 1 = 2-bit
     );

  //now we have to initialize our transmit and receive buffers
  CBUF_INIT(&usart1_tx, tx_buf, DEFAULT_BUF_SIZE);
  CBUF_INIT(&usart1_rx, rx_buf, DEFAULT_BUF_SIZE);
  */
}


//the code on page 179 of the ATMega128 datasheet
//  has a simple send routine
void USART1_send_byte(int8_t data) {
  //if we are supposed to send a newline '\n', we
  //need to add a carriage return as well
  //to make things look pretty for terminals
  if(data == '\n') {
    while(CBUF_FULL(&usart1_tx)) {}
    CBUF_PUSH(&usart1_tx, '\r');
  }
  //push the data byte into the transmit buffer
  //first we have to wait till the transmit buffer has space
  while(CBUF_FULL(&usart1_tx)) {}
  CBUF_PUSH(&usart1_tx, data);

  //now we enable the USART data register empty interrupt
  //this will allow us to send data without having to
  //poll 
  UCSR1B |= (1<<UDRIE1);
}

//used for fdevopen
int USART1_stdio_send(char data, FILE * f) {
  USART1_send_byte(data);
  return 0;
}

//check to see if the input buffer contains a newline character
//  This will tell us if gets, fgets and whatnot will return or block
int USART1_contains_newline() {
  int len = CBUF_LEN(&usart1_rx);

  int i; char c;

  for(i = 0; i < len; i++) {
    c = CBUF_GET(&usart1_rx, i);
    if(c == '\r' || c == '\n') {
      return 1;
    }
  }

  return 0;
}

//We have a command if the buffer is either full or contains a newline
//  AND we've parsed out the command line to figure out what we want to do.
int USART1_input_ready() {
 	 return CBUF_FULL(&usart1_rx) || USART1_contains_newline();
}


//Returns the current input
uint8_t* USART1_get_input() {
	int idx = 0;
	for (; idx < CBUF_LEN(&usart1_rx) && idx < INPUT_BUFFER_SIZE; idx++)
		usart_input[idx] = CBUF_GET(&usart1_rx, idx);

	usart_input[idx++] = '\0';

	while(!CBUF_EMPTY(&usart1_rx))
		CBUF_POP(&usart1_rx);

	return (uint8_t*)&usart_input;
}

//check if there is a byte available in the receive
//buffer.  This will tell if get_byte will block or
//not
int USART1_byte_available() {
  return  ! CBUF_EMPTY(&usart1_rx);
}

//if there is no byte available, block!
//  check with byte_available if you need non-blocking
int8_t USART1_get_byte() {
  while(CBUF_EMPTY(&usart1_rx)) {}
  
  uint8_t ret = CBUF_POP(&usart1_rx);

  return ret;
}

//for use with fdevopen
int USART1_stdio_get(FILE * f) {
  return USART1_get_byte();
}


/**********************************************************
 *  SIGNAL HANDLERS
 *  
 *  this is where the actual sending and recieving of data
 *  happens
 *
 *  see page 59 for available interrupts
 *  signal names defined in avr/iom128.h
 **********************************************************/
/*
//called when there is data to receive
ISR(USART1_RX_vect) {

	G_flag = 1;

  //the data is stored in the UDR register
  uint8_t ch = UDR1;

  //if we got a character return '\r' we need to
  //replace it with a newline (for stdio functions)
  if(ch == '\r') ch = '\n';

lcd_goto_xy(0,0);
print_character(ch);

  //If we have a backspace character and the buffer isn't empty,
  //we need to remove the most-recently added char from the buffer.
  if (ch == 127 && ! CBUF_EMPTY(&usart1_rx)) { //backspace
  	CBUF_UNPUSH(&usart1_rx);
	return;
  }
  
  //if the recieve buffer is full, we lose the data
  if( ! CBUF_FULL(&usart1_rx)) {
    CBUF_PUSH(&usart1_rx, ch);
  }
}
*/
/*
//called with the data register is empty
//  the data register is where we put the data we want 
//  to send.
ISR(USART1_TX_vect) {
  if(! CBUF_EMPTY(&usart1_tx)) {
    //pop data from the transmit buffer and put it in the
    //data register

    uint8_t ch = CBUF_POP(&usart1_tx);
    UDR1 = ch;
  } else {
    //nothing to send, so turn off the interrupt
    UCSR1B &= ~(1 << UDRIE1);
  }
}
*/

void check_for_new_bytes_received()
{
    while(serial_get_received_bytes(USB_COMM) != receive_buffer_position)
    {
        // Process the new byte that has just been received.
        print_character(receive_buffer[receive_buffer_position]);
        //        process_received_byte(receive_buffer[receive_buffer_position]);

        // Increment receive_buffer_position, but wrap around when it gets to
        // the end of the buffer.
        if (receive_buffer_position == sizeof(receive_buffer)-1)
        {
            receive_buffer_position = 0;
        }
        else
        {
            receive_buffer_position++;
        }
    }
}

void usart_check()
{
    clear();
    lcd_goto_xy(0,1);
    serial_check();
    print_long(serial_get_received_bytes(USB_COMM));
    check_for_new_bytes_received();
}
