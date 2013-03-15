/* timer_1284p.c
 *
 * Created: 3/10/2013 10:01:25 PM
 *  Author: Kyle
 */

#ifndef __TIMER_1284P_H
#define __TIMER_1284P_H

#include "timer_1284p.h"
#include <pololu/orangutan.h>

void timer_1284p_set_COM(TIMER_1284P_E timer, TIMER_1284P_AB_E ab, TIMER_1284P_COM_E com)
{
    int reg_val;
    int shift_amount;
    int mask;

    reg_val = com & 0x3;
    shift_amount = 0;

    switch ( ab )
    {
        case TIMER_1284P_A:
            shift_amount = COM0A0;
            mask = (1<<COM0A1) | (1<<COM0A0);
            break;
        case TIMER_1284P_B:
            shift_amount = COM0B0;
            mask = (1<<COM0B1) | (1<<COM0B0);
            break;
        default:
            reg_val = 0;
            break;
    }

    reg_val <<= shift_amount;

    switch( timer )
    {
        case TIMER_1284P_0:
            TCCR0A &= mask;
            TCCR0A |= reg_val;
            break;
        case TIMER_1284P_1:
            TCCR1A &= mask;
            TCCR1A |= reg_val;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            TCCR3A &= mask;
            TCCR3A |= reg_val;
            break;
        default:
            break;
    }
}

void timer_1284p_set_WGM(TIMER_1284P_E timer, TIMER_1284P_WGM_E wgm)
{
    int reg_val_a, reg_val_b;
    int mask_a;
    int mask_b;

    switch( timer )
    {
        case TIMER_1284P_0:
            switch ( wgm )
            {
                case TIMER_1284P_WGM_NORMAL:
                    reg_val_a = (0<<WGM00) | (0<<WGM01);
                    reg_val_b = (0<<WGM02);
                    break;
                case TIMER_1284P_WGM_CTC:
                    reg_val_a = (0<<WGM00) | (1<<WGM01);
                    reg_val_b = (0<<WGM02);
                    break;
                default:
                    reg_val_a = 0;
                    reg_val_b = 0;
                    break;
            }

            mask_a = (1<<WGM00) | (1<<WGM01);
            mask_b = (1<<WGM02);

            TCCR0A &= ~mask_a;
            TCCR0A |= reg_val_a;
            TCCR0B &= ~mask_b;
            TCCR0B |= reg_val_b;
            break;
        case TIMER_1284P_1:
            switch ( wgm )
            {
                case TIMER_1284P_WGM_NORMAL:
                    reg_val_a = (0<<WGM13) | (0<<WGM11);
                    reg_val_b = (0<<WGM12) | (0<<WGM13);
                    break;
                case TIMER_1284P_WGM_CTC:
                    reg_val_a = (0<<WGM10) | (0<<WGM11);
                    reg_val_b = (1<<WGM12) | (0<<WGM13);
                    break;
                default:
                    reg_val_a = 0;
                    reg_val_b = 0;
                    break;
            }

            mask_a = (1<<WGM10) | (1<<WGM11);
            mask_b = (1<<WGM12) | (1<<WGM13);

            TCCR1A &= ~mask_a;
            TCCR1A |= reg_val_a;
            TCCR1B &= ~mask_b;
            TCCR1B |= reg_val_b;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            switch ( wgm )
            {
                case TIMER_1284P_WGM_NORMAL:
                    reg_val_a = (0<<WGM33) | (0<<WGM31);
                    reg_val_b = (0<<WGM32) | (0<<WGM33);
                    break;
                case TIMER_1284P_WGM_CTC:
                    reg_val_a = (0<<WGM30) | (0<<WGM31);
                    reg_val_b = (1<<WGM32) | (0<<WGM33);
                    break;
                default:
                    reg_val_a = 0;
                    reg_val_b = 0;
                    break;
            }

            mask_a = (1<<WGM30) | (1<<WGM31);
            mask_b = (1<<WGM32) | (1<<WGM33);

            TCCR3A &= ~mask_a;
            TCCR3A |= reg_val_a;
            TCCR3B &= ~mask_b;
            TCCR3B |= reg_val_b;
            break;
        default:
            break;
    }
}

void timer_1284p_set_CS(TIMER_1284P_E timer, TIMER_1284P_CS_E cs)
{
    int reg_val;
    int mask;

    reg_val = ( cs & 0x7 ) << CS00;
    mask = (1<<CS00) | (1<<CS01) | (1<<CS02);

    switch( timer )
    {
        case TIMER_1284P_0:
            TCCR0B &= ~mask;
            TCCR0B |= reg_val;
            break;
        case TIMER_1284P_1:
            TCCR1B &= ~mask;
            TCCR1B |= reg_val;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            TCCR3B &= ~mask;
            TCCR3B |= reg_val;
            break;
        default:
            break;
    }
}

void timer_1284p_set_OCR(TIMER_1284P_E timer, TIMER_1284P_AB_E ab, int duration_counts)
{
    switch( timer )
    {
        case TIMER_1284P_0:
            switch ( ab )
            {
                case TIMER_1284P_A:
                    OCR0A = duration_counts;
                    break;
                case TIMER_1284P_B:
                    OCR0B = duration_counts;
                    break;
                default:
                    break;
            }
            break;
        case TIMER_1284P_1:
            switch ( ab )
            {
                case TIMER_1284P_A:
                    OCR1A = duration_counts;
                    break;
                case TIMER_1284P_B:
                    OCR1B = duration_counts;
                    break;
                default:
                    break;
            }
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            switch ( ab )
            {
                case TIMER_1284P_A:
                    OCR3A = duration_counts;
                    break;
                case TIMER_1284P_B:
                    OCR3B = duration_counts;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void timer_1284p_set_IE(TIMER_1284P_E timer, TIMER_1284P_INT_E interrupt)
{
    int reg_value;

    switch ( interrupt )
    {
        case TIMER_1284P_IE_B:
            reg_value = (1<<OCIE0B);
            break;
        case TIMER_1284P_IE_A:
            reg_value = (1<<OCIE0A);
            break;
        case TIMER_1284P_IE_OVERFLOW:
            reg_value = (1<<TOIE0);
            break;
        default:
            reg_value = 0;
            break;
    }

    switch( timer )
    {
        case TIMER_1284P_0:
            TIMSK0 |= reg_value;
            break;
        case TIMER_1284P_1:
            TIMSK1 |= reg_value;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            TIMSK3 |= reg_value;
            break;
        default:
            break;
    }
}

void timer_1284p_clr_counter(TIMER_1284P_E timer)
{
    switch( timer )
    {
        case TIMER_1284P_0:
            TCNT0 = 0;
            break;
        case TIMER_1284P_1:
            TCNT1 = 0;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            TCNT3 = 0;
            break;
        default:
            break;
    }
}

void timer_1284p_clr_IE(TIMER_1284P_E timer, TIMER_1284P_INT_E interrupt)
{
    int reg_value;

    switch ( interrupt )
    {
        case TIMER_1284P_IE_B:
            reg_value = (1<<OCIE0B);
            break;
        case TIMER_1284P_IE_A:
            reg_value = (1<<OCIE0A);
            break;
        case TIMER_1284P_IE_OVERFLOW:
            reg_value = (1<<TOIE0);
            break;
        default:
            reg_value = 0;
            break;
    }

    switch( timer )
    {
        case TIMER_1284P_0:
            TIMSK0 &= ~reg_value;
            break;
        case TIMER_1284P_1:
            TIMSK1 &= ~reg_value;
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
            TIMSK3 &= ~reg_value;
            default:
            break;
    }
}

int timer_1284p_get_counter(TIMER_1284P_E timer)
{
    int ret_value;

    ret_value = 0;

    switch( timer )
    {
    case TIMER_1284P_0:
        ret_value = TCNT0;
        break;
    case TIMER_1284P_1:
        ret_value = TCNT1;
        break;
    case TIMER_1284P_2:
        break;
    case TIMER_1284P_3:
        ret_value = TCNT3;
        break;
    default:
        break;
    }

    return ret_value;
}

#endif
