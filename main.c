#include <avr/io.h>
#include <avr/delay.h>
#include <stdint.h>

uint8_t space[] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

uint8_t A[] = {
    0b00111100,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01111110,
    0b01000010,
    0b01000010,
    0b01000010
};

uint8_t L[] = {
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01000000,
    0b01111110
};

uint8_t O[] = {
    0b00111100,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01000010,
    0b00111100
};

uint8_t H[] = {
    0b01000010,
    0b01000010,
    0b01000010,
    0b01111110,
    0b01000010,
    0b01000010,
    0b01000010,
    0b01000010
};


/* Shift Register Lines:
 * SRCLK: PB4 = Load next LED bit
 * RCLK:  PB3 = Apply loaded byte to LED's
 * !OE:   GND
 * SER:   PB2 = data bit to shift into register
 *
 * Shift Register Columns:
 * SRCLK: PB1
 * RCLK:  PB1
 * !OE:   GND
 * SER:   PB0
 */

uint8_t* currently_drawing = A;

uint8_t state = 0;

void draw_isr(void) {
    uint8_t line = (state>>1)&0b111;
    uint8_t column = (state>>4)&0b111;
    if(~state&1) { // state is even -> set bit to load
        if(currently_drawing[line]&(1<<column)) {
            PORTB |= (1<<PB2);
        }
        else {
            PORTB &= ~(1<<PB2);
        }
        PORTB |= (1<<PB4); // load bit
        if(line==7)
            PORTB |= (1<<PB3); // apply
        if(column==7)
            PORTB |= (1<<PB0);
        else
            PORTB &= ~(1<<PB0);
        if(line==7)
            PORTB |= (1<<PB1);

    }
    else {
        PORTB &= ~(1<<PB4);
        PORTB &= ~(1<<PB3);
        if(line==7)
            PORTB &= ~(1<<PB1);
    }
    state += 1;
}

int main(void)
{

    DDRB |= (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4);

    uint8_t* string[] = {H, A, L, L, O, space};

    while(1) {
        for(uint8_t i=0; i<6; i++) {
            currently_drawing = string[i];
            for(uint16_t j=0; j<5000; j++) {
                draw_isr();
                _delay_us(100);
            }
            currently_drawing = space;
            for(uint16_t j=0; j<1000; j++) {
                draw_isr();
                _delay_us(100);
            }
        }
    }
    
    return 0;
}
