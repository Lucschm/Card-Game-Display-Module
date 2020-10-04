#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

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

uint8_t nothing[] = {
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101,
    0b10101010,
    0b01010101
};

uint8_t _7[] = {
    0b01111110,
    0b00000010,
    0b00000100,
    0b00001000,
    0b00010000,
    0b00100000,
    0b01000000,
    0b01000000
};

uint8_t _8[] = {
    0b00111100,
    0b01000010,
    0b01000010,
    0b00111100,
    0b01000010,
    0b01000010,
    0b01000010,
    0b00111100
};

uint8_t _9[] = {
    0b00111100,
    0b01000010,
    0b01000010,
    0b00111110,
    0b00000010,
    0b00000010,
    0b01000100,
    0b00111000
};

uint8_t _10[] = {
    0b00000000,
    0b10011110,
    0b10100001,
    0b10100001,
    0b10100001,
    0b10100001,
    0b10100001,
    0b10011110
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

uint8_t K[] = {
    0b01000100,
    0b01001000,
    0b01010000,
    0b01100000,
    0b01100000,
    0b01010000,
    0b01001000,
    0b01000100
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
    0b10000001,
    0b10000001,
    0b10000001,
    0b10000001,
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

uint8_t U[] = {
    0b10000010,
    0b10000010,
    0b10000010,
    0b10000010,
    0b10000010,
    0b10000010,
    0b01000100,
    0b00111000
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
 *
 * Button:
 * PC7
 */

uint8_t* currently_drawing = space;

uint8_t state = 0;
uint8_t brightness = 2; // max 2
uint8_t br_ctr=0;

uint8_t rotation = 3;
uint16_t draw_ctr = 0;

void draw_isr(void) {
    uint8_t line = (state>>1)&0b111;
    uint8_t column = (state>>4)&0b111;
    uint8_t _line = 0;
    uint8_t _column = 0;
    rotation = (draw_ctr>>8)&0b11;
    switch(rotation) {
        case 0:
            _line = line;
            _column = column;
            break;
        case 1:
            _line = column;
            _column = 7-line;
            break;
        case 2:
            _line = 7-line;
            _column = 7-column;
            break;
        case 3:
            _line = 7-column;
            _column = line;
            break;
    }

    if(~state&1) { // state is even -> set bit to load
        if(currently_drawing[_line]&(1<<_column) && br_ctr<brightness) {
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
    if(state>=128) {
        state = 0;
        br_ctr++;
        draw_ctr++;
        if(br_ctr>2)
            br_ctr=0;
    }
}

ISR(TIMER0_OVF_vect)
{
    draw_isr();
}

struct card {
    uint8_t* symbol;
    struct card* next;
};
typedef struct card card;

int main(void)
{
    DDRB |= (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4);
    DDRC &= ~(1<<PC7); // Button
    PORTC |= (1<<PC7);


    TIMSK |= (1<<TOIE0);                    // Enable Timer 0 Overflow interrupt
    TCCR0 |= (1<<CS00)|(0<<CS02);           // Start timer0 with prescaler 1

    sei();                                  // Set the I-bit in SREG


    // Wait for first button press to seed randomness
    currently_drawing = nothing;
    while(PINC&(1<<PC7));
    _delay_ms(50);
    while(~PINC&(1<<PC7));
    _delay_ms(50);
    srand(TCNT0);

    card deck_start;
    deck_start.symbol = NULL;
    deck_start.next = NULL;

    uint8_t* symbols[] = {_7, _8, _9, _10, U,O,K,A,_7, _8, _9, _10, U,O,K,A,_7, _8, _9, _10, U,O,K,A,_7, _8, _9, _10, U,O,K,A,NULL};
    card* previous = &deck_start;
    for(uint8_t i=0; symbols[i]; i++) {
        card* mycard = (card*)malloc(sizeof(card));
        mycard->symbol = symbols[i];
        mycard->next = NULL;
        previous->next = mycard;
        previous = mycard;
    }
    previous->next = deck_start.next;

    card* draw_next = previous;
    for(uint8_t i=0; i<rand()%8; i++)
        draw_next = draw_next->next;

    uint8_t kine_ctr = 0;

    card* mixed_start = draw_next->next;
    draw_next->next = draw_next->next->next;
    card* mixed_tail = mixed_start;

    currently_drawing = mixed_tail->symbol;
    if(mixed_tail->symbol == K)
        kine_ctr++;
    while(mixed_tail->next) {
        while(PINC&(1<<PC7));
        _delay_ms(50);
        while(~PINC&(1<<PC7));
        _delay_ms(50);
        srand(TCNT0);

        for(uint8_t i=0; i<rand()%8; i++)
            draw_next = draw_next->next;
        mixed_tail->next = draw_next->next;
        mixed_tail = mixed_tail->next;
        draw_next->next = draw_next->next->next; //wtf
        if(draw_next == draw_next->next) {
            mixed_tail->next = draw_next;
            mixed_tail = mixed_tail->next;
            mixed_tail->next = NULL;
        }

        currently_drawing = space;
        _delay_ms(100);
        currently_drawing = mixed_tail->symbol;
        if(mixed_tail->symbol == K)
            kine_ctr++;

        if(kine_ctr>=4) {
            while(1) {
                currently_drawing = nothing;
                _delay_ms(150);
                currently_drawing = K;
                _delay_ms(150);
            }
        }
    }

    while(1) {
        currently_drawing = nothing;
        _delay_ms(150);
        currently_drawing = space;
        _delay_ms(150);
    }
    
    return 0;
}
