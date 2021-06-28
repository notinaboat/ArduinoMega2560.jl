//==============================================================================
// AVR GPIO
// 
// Copyright OC Technology Pty Ltd 2021.
//==============================================================================

#ifndef AVR_GPIO_INCLUDED
#define AVR_GPIO_INCLUDED


uint8_t read_input(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': return PINA & mask;
        case 'B': return PINB & mask;
        case 'C': return PINC & mask;
        case 'D': return PIND & mask;
        #ifdef PINE
        case 'E': return PINE & mask;
        case 'F': return PINF & mask;
        case 'G': return PING & mask;
        case 'H': return PINH & mask;
        case 'J': return PINJ & mask;
        case 'K': return PINK & mask;
        case 'L': return PINL & mask;
        #endif
        default: assert(0, "Bad GPIO Port!");
    }
}


void enable_input_with_pullup(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA &= ~mask; PORTA |= mask ; break;
        case 'B': DDRB &= ~mask; PORTB |= mask ; break;
        case 'C': DDRC &= ~mask; PORTC |= mask ; break;
        case 'D': DDRD &= ~mask; PORTD |= mask ; break;
        #ifdef PINE
        case 'E': DDRE &= ~mask; PORTE |= mask ; break;
        case 'F': DDRF &= ~mask; PORTF |= mask ; break;
        case 'G': DDRG &= ~mask; PORTG |= mask ; break;
        case 'H': DDRH &= ~mask; PORTH |= mask ; break;
        case 'J': DDRJ &= ~mask; PORTJ |= mask ; break;
        case 'K': DDRK &= ~mask; PORTK |= mask ; break;
        case 'L': DDRL &= ~mask; PORTL |= mask ; break;
        #endif
        default: assert(0, "Bad GPIO Port!");
    }
}


void enable_input_without_pullup(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA &= ~mask; PORTA &= ~mask ; break;
        case 'B': DDRB &= ~mask; PORTB &= ~mask ; break;
        case 'C': DDRC &= ~mask; PORTC &= ~mask ; break;
        case 'D': DDRD &= ~mask; PORTD &= ~mask ; break;
        #ifdef PINE
        case 'E': DDRE &= ~mask; PORTE &= ~mask ; break;
        case 'F': DDRF &= ~mask; PORTF &= ~mask ; break;
        case 'G': DDRG &= ~mask; PORTG &= ~mask ; break;
        case 'H': DDRH &= ~mask; PORTH &= ~mask ; break;
        case 'J': DDRJ &= ~mask; PORTJ &= ~mask ; break;
        case 'K': DDRK &= ~mask; PORTK &= ~mask ; break;
        case 'L': DDRL &= ~mask; PORTL &= ~mask ; break;
        #endif
        default: assert(0, "Bad GPIO Port!");
    }
}


void output_high(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA |= mask; PORTA |= mask ; break;
        case 'B': DDRB |= mask; PORTB |= mask ; break;
        case 'C': DDRC |= mask; PORTC |= mask ; break;
        case 'D': DDRD |= mask; PORTD |= mask ; break;
        #ifdef PINE
        case 'E': DDRE |= mask; PORTE |= mask ; break;
        case 'F': DDRF |= mask; PORTF |= mask ; break;
        case 'G': DDRG |= mask; PORTG |= mask ; break;
        case 'H': DDRH |= mask; PORTH |= mask ; break;
        case 'J': DDRJ |= mask; PORTJ |= mask ; break;
        case 'K': DDRK |= mask; PORTK |= mask ; break;
        case 'L': DDRL |= mask; PORTL |= mask ; break;
        #endif
        default: assert(0, "Bad GPIO Port!");
    }
}


void output_low(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA |= mask; PORTA &= ~mask ; break;
        case 'B': DDRB |= mask; PORTB &= ~mask ; break;
        case 'C': DDRC |= mask; PORTC &= ~mask ; break;
        case 'D': DDRD |= mask; PORTD &= ~mask ; break;
        #ifdef PINE
        case 'E': DDRE |= mask; PORTE &= ~mask ; break;
        case 'F': DDRF |= mask; PORTF &= ~mask ; break;
        case 'G': DDRG |= mask; PORTG &= ~mask ; break;
        case 'H': DDRH |= mask; PORTH &= ~mask ; break;
        case 'J': DDRJ |= mask; PORTJ &= ~mask ; break;
        case 'K': DDRK |= mask; PORTK &= ~mask ; break;
        case 'L': DDRL |= mask; PORTL &= ~mask ; break;
        #endif
        default: assert(0, "Bad GPIO Port!");
    }
}


#ifdef ADCSRB
uint16_t analog_input(const uint8_t port, const uint8_t pin)
{
    // Enable the ADC.
    ADCSRA |= bit1(ADEN);

    // Use AVCC as reference.
    ADMUX = bit1(REFS0) | pin;

    switch(port) {
        case 'F': ADCSRB &= ~bit1(MUX5) ; break;
        case 'K': ADCSRB |=  bit1(MUX5) ; break;
        default: assert(0, "Bad ADC Port!");
    }

    // Start the conversion.
    ADCSRA |= bit1(ADSC);

    // Wait for the conversion to complete...
    while (ADCSRA & bit1(ADSC));

    return ADC;
}
#endif



#endif // AVR_GPIO_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
