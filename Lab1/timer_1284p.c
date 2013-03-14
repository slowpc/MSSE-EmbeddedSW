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
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
        case TIMER_1284P_1:
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
            break;
        case TIMER_1284P_2:
            break;
        case TIMER_1284P_3:
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
        break;
    case TIMER_1284P_2:
        break;
    case TIMER_1284P_3:
        break;
    default:
        break;
    }

    return ret_value;
}

#endif
