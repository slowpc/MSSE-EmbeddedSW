/* two_rotations - an application for the Pololu Orangutan SVP
 *
 * This application uses the Pololu AVR C/C++ Library.  For help, see:
 * -User's guide: http://www.pololu.com/docs/0J20
 * -Command reference: http://www.pololu.com/docs/0J18
 *
 * Created: 3/1/2013 3:23:03 PM
 *  Author: Kyle
 */

// Includes
#include <stdio.h>
#include <string.h>
#include <pololu/orangutan.h>
#include <pololu/OrangutanPushbuttons/OrangutanPushbuttons.h>


// Defines for the system


// Motor
#define MOTOR_SPEED_INIT                50
#define MOTOR_SPEED_BUTTON_DELTA        25
#define MOTOR_SPEED_STOP                0
#define MOTOR_SPEED_MIN                 0
#define MOTOR_SPEED_MAX                 125

// Encoder
#define ENCODER_COUNT_HYS_RANGE         8
#define ENCODER_COUNT_PER_REV           32
#define ENCODER_NUM_REVS                2
#define ENCODER_ABS_MAX                 ( ENCODER_COUNT_PER_REV * ENCODER_NUM_REVS )
#define ENCODER_MIN                     -ENCODER_ABS_MAX
#define ENCODER_MAX                     ENCODER_ABS_MAX
#define ENCODER_START                   -ENCODER_ABS_MAX

// Timing
#define TIME_DELAY                      50

// LCD Count
#define COUNT_STRING                    "count: "
#define LCD_ROW_COUNT                   0
#define LCD_COL_COUNT                   0

// LCD Speed
#define SPEED_STRING                    "speed: "
#define LCD_ROW_SPEED                   1
#define LCD_COL_SPEED                   0

// Encoder error
#define ERROR_SET_STRING                "E"
#define ERROR_CLEAR_STRING              " "
#define LCD_ROW_ENCODER_ERROR_LATCH     0
#define LCD_ROW_ENCODER_ERROR           1
#define LCD_COL_ENCODER_ERROR           14

// Motor disable
#define MOTOR_DISABLE_STRING            "X"
#define MOTOR_DISABLE_CLEAR_STRING      " "
#define LCD_ROW_MOTOR_DISABLE           0
#define LCD_COL_MOTOR_DISABLE           15

// Speed
#define SPEED_UP_STRING                 "U"
#define SPEED_DOWN_STRING               "D"
#define SPEED_CLEAR_STRING              " "
#define LCD_ROW_SPEED_SETTING           1
#define LCD_COL_SPEED_SETTING           15

// Button mapping
#define BUTTON_DISABLE_MOTOR            TOP_BUTTON
#define BUTTON_SPEED_UP                 MIDDLE_BUTTON
#define BUTTON_SPEED_DOWN               BOTTOM_BUTTON

// Encoder pin mapping
#define PIN_ENCODER_1A                  IO_A0
#define PIN_ENCODER_1B                  IO_A1
#define PIN_ENCODER_2A                  IO_A2
#define PIN_ENCODER_2B                  IO_A3


// Enums

typedef enum
{
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD,
} DIRECTION_E;


int main()
{
    // Declare inputs
    unsigned char button_dbc_press, button_dbc_release, button_pressed;
    int motor_speed_output, motor_speed_magnitude, motor_speed_stored, motor_speed_req;
    int count_value, count_error;
    int str_len_count, str_len_speed;
    int motor_disable, motor_enable, motor_no_change, speed_up, speed_down, speed_no_change;
    DIRECTION_E direction;

    lcd_init_printf();
    clear();

    // Display these once, don't need to continuously write them
    lcd_goto_xy( LCD_COL_COUNT, LCD_ROW_COUNT );
    print( COUNT_STRING );
    lcd_goto_xy( LCD_COL_SPEED, LCD_ROW_SPEED );
    print( SPEED_STRING );

    // Calculate these constants once
    str_len_count = strlen( COUNT_STRING );
    str_len_speed = strlen( SPEED_STRING );

    // Initialize the encoders and specify the four input pins, first two are for motor 1, second two are for motor 2
    encoders_init( PIN_ENCODER_1A, PIN_ENCODER_1B, PIN_ENCODER_2A, PIN_ENCODER_2B );

    // Initialize the motor speed and print
    motor_speed_magnitude = MOTOR_SPEED_INIT;
    motor_speed_stored = motor_speed_magnitude;
    motor_speed_output = motor_speed_magnitude;

    // Default direction state on power up
    direction = DIRECTION_FORWARD;

    while( 1 )
    {

        // Button inputs
        button_pressed          = button_is_pressed( ALL_BUTTONS );
        button_dbc_press        = get_single_debounced_button_press( ALL_BUTTONS );
        button_dbc_release      = get_single_debounced_button_release( ALL_BUTTONS );

        // Count inputs
        count_value             = encoders_get_counts_m2() + ENCODER_START;
        count_error             = encoders_check_error_m2();

        // Input Logic
        motor_disable           =  ( button_dbc_press   & BUTTON_DISABLE_MOTOR                      ) == BUTTON_DISABLE_MOTOR;
        motor_enable            =  ( button_dbc_release & BUTTON_DISABLE_MOTOR                      ) == BUTTON_DISABLE_MOTOR;
        motor_no_change         =  ( button_pressed     & BUTTON_DISABLE_MOTOR                      ) == 0;
        speed_up                =  ( button_dbc_press   & BUTTON_SPEED_UP                           ) == BUTTON_SPEED_UP;
        speed_down              =  ( button_dbc_press   & BUTTON_SPEED_DOWN                         ) == BUTTON_SPEED_DOWN;
        speed_no_change         =  ( button_pressed     & ( BUTTON_SPEED_UP | BUTTON_SPEED_DOWN )   ) == 0;

        // Print count
        lcd_goto_xy( str_len_count, LCD_ROW_COUNT );
        printf("%5d", count_value);

        // Print count error information
        if( count_error )
        {
            // Latch the error
            lcd_goto_xy( LCD_COL_ENCODER_ERROR, LCD_ROW_ENCODER_ERROR_LATCH );
            print( ERROR_SET_STRING );
            
            // Set error
            lcd_goto_xy( LCD_COL_ENCODER_ERROR, LCD_ROW_ENCODER_ERROR );
            print( ERROR_SET_STRING );
        }
        else
        {
            // Clear error
            lcd_goto_xy( LCD_COL_ENCODER_ERROR, LCD_ROW_ENCODER_ERROR );
            print( ERROR_CLEAR_STRING );
        }

        // Motor logic
        if ( motor_disable )
        {
            // Disable motor button pressed (edge), disable the motor

            // Save the current speed and then stop
            motor_speed_stored = motor_speed_magnitude;
            motor_speed_magnitude = MOTOR_SPEED_STOP;
            lcd_goto_xy( LCD_COL_MOTOR_DISABLE, LCD_ROW_MOTOR_DISABLE );
            print( MOTOR_DISABLE_STRING );
        }
        else if ( motor_enable )
        {
            // Disable motor button is released (edge), enable the motor

            // Resume the saved speed
            motor_speed_magnitude = motor_speed_stored;
        }
        else if ( motor_no_change )
        {
            // Disable motor button is not pressed
            lcd_goto_xy( LCD_COL_MOTOR_DISABLE, LCD_ROW_MOTOR_DISABLE );
            print( MOTOR_DISABLE_CLEAR_STRING );
        }

        // Speed logic
        if ( speed_up )
        {
            // Speed up button pressed (edge), speed up the motor (to a limit)
            motor_speed_req = motor_speed_magnitude + MOTOR_SPEED_BUTTON_DELTA;
            if ( motor_speed_req <= MOTOR_SPEED_MAX )
            {
                motor_speed_magnitude = motor_speed_req;
            }
            lcd_goto_xy( LCD_COL_SPEED_SETTING, LCD_ROW_SPEED_SETTING );
            print( SPEED_UP_STRING );
        }
        else if ( speed_down )
        {
            // Speed up button pressed (edge), slow down the motor (to a limit)
            motor_speed_req = motor_speed_magnitude - MOTOR_SPEED_BUTTON_DELTA;
            if ( motor_speed_req >= MOTOR_SPEED_MIN )
            {
                motor_speed_magnitude = motor_speed_req;
            }
            lcd_goto_xy( LCD_COL_SPEED_SETTING, LCD_ROW_SPEED_SETTING );
            print( SPEED_DOWN_STRING );
        }
        else if ( speed_no_change )
        {
            // All speed buttons are not pressed
            lcd_goto_xy( LCD_COL_SPEED_SETTING, LCD_ROW_SPEED_SETTING );
            print( SPEED_CLEAR_STRING );
        }

        // New motor output
        if ( count_value > ENCODER_MAX )
        {
            direction = DIRECTION_BACKWARD;
            motor_speed_output = -motor_speed_magnitude;
        }
        else if ( count_value < ENCODER_MIN )
        {
            direction = DIRECTION_FORWARD;
            motor_speed_output = motor_speed_magnitude;
        }
        else
        {
            if ( direction == DIRECTION_FORWARD )
            {
                // Normal operation, full forward
                motor_speed_output = motor_speed_magnitude;
            }
            else
            {
                // Normal operation, full backward
                motor_speed_output = -motor_speed_magnitude;
            }
        }

        // Set motor outputs
        set_motors( motor_speed_output, motor_speed_output );
        lcd_goto_xy( str_len_speed, LCD_ROW_SPEED );
        printf("%5d", motor_speed_output );

        // Delay
        delay_ms( TIME_DELAY );
    }
}
