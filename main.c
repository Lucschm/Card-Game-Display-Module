#include <avr/io.h>
#include <avr/delay.h>

int main(void)
{
    DDRB |= (1<<0) | (1<<1);
    
    
    while(1) {
        PORTB |= (1<<0); // set SER
        PORTB ^= (1<<1); // toggle RCLK and SRCLK
        PORTB &= ~(1<<0); // unset SER
        for(int i=0; i<8; i++) {
            _delay_ms(100);
            PORTB ^= (1<<1); // toggle RCLK and SRCLK
        }
    }
    
    return 0;
}
