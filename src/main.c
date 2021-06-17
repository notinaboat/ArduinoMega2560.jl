//==============================================================================
// Arduino Mega 2560 Julia Interface Firmware
//
// See ArduinoMega2560.jl
// 
// Copyright OC Technology Pty Ltd 2021.
//==============================================================================


#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "assert.h"
#include "bit.h"

#include "avr_usart0.h"
#include "fifo.h"
#include "output.h"
#include "input.h"

static uint8_t read_input(uint8_t port, uint8_t pin)
{
    uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': return PINA & mask;
        case 'B': return PINB & mask;
        case 'C': return PINC & mask;
        case 'D': return PIND & mask;
        case 'E': return PINE & mask;
        case 'F': return PINF & mask;
        case 'G': return PING & mask;
        case 'H': return PINH & mask;
        case 'J': return PINJ & mask;
        case 'K': return PINK & mask;
        case 'L': return PINL & mask;
        default: assert(0);
    }
}

static void enable_input_with_pullup(uint8_t port, uint8_t pin)
{
    uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA &= ~mask; PORTA |= mask ; break;
        case 'B': DDRB &= ~mask; PORTB |= mask ; break;
        case 'C': DDRC &= ~mask; PORTC |= mask ; break;
        case 'D': DDRD &= ~mask; PORTD |= mask ; break;
        case 'E': DDRE &= ~mask; PORTE |= mask ; break;
        case 'F': DDRF &= ~mask; PORTF |= mask ; break;
        case 'G': DDRG &= ~mask; PORTG |= mask ; break;
        case 'H': DDRH &= ~mask; PORTH |= mask ; break;
        case 'J': DDRJ &= ~mask; PORTJ |= mask ; break;
        case 'K': DDRK &= ~mask; PORTK |= mask ; break;
        case 'L': DDRL &= ~mask; PORTL |= mask ; break;
        default: assert(0);
    }
}


static void enable_input_without_pullup(uint8_t port, uint8_t pin)
{
    uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA &= ~mask; PORTA &= ~mask ; break;
        case 'B': DDRB &= ~mask; PORTB &= ~mask ; break;
        case 'C': DDRC &= ~mask; PORTC &= ~mask ; break;
        case 'D': DDRD &= ~mask; PORTD &= ~mask ; break;
        case 'E': DDRE &= ~mask; PORTE &= ~mask ; break;
        case 'F': DDRF &= ~mask; PORTF &= ~mask ; break;
        case 'G': DDRG &= ~mask; PORTG &= ~mask ; break;
        case 'H': DDRH &= ~mask; PORTH &= ~mask ; break;
        case 'J': DDRJ &= ~mask; PORTJ &= ~mask ; break;
        case 'K': DDRK &= ~mask; PORTK &= ~mask ; break;
        case 'L': DDRL &= ~mask; PORTL &= ~mask ; break;
        default: assert(0);
    }
}

static void output_high(uint8_t port, uint8_t pin)
{
    uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA |= mask; PORTA |= mask ; break;
        case 'B': DDRB |= mask; PORTB |= mask ; break;
        case 'C': DDRC |= mask; PORTC |= mask ; break;
        case 'D': DDRD |= mask; PORTD |= mask ; break;
        case 'E': DDRE |= mask; PORTE |= mask ; break;
        case 'F': DDRF |= mask; PORTF |= mask ; break;
        case 'G': DDRG |= mask; PORTG |= mask ; break;
        case 'H': DDRH |= mask; PORTH |= mask ; break;
        case 'J': DDRJ |= mask; PORTJ |= mask ; break;
        case 'K': DDRK |= mask; PORTK |= mask ; break;
        case 'L': DDRL |= mask; PORTL |= mask ; break;
        default: assert(0);
    }
}

static void output_low(uint8_t port, uint8_t pin)
{
    uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': DDRA |= mask; PORTA &= ~mask ; break;
        case 'B': DDRB |= mask; PORTB &= ~mask ; break;
        case 'C': DDRC |= mask; PORTC &= ~mask ; break;
        case 'D': DDRD |= mask; PORTD &= ~mask ; break;
        case 'E': DDRE |= mask; PORTE &= ~mask ; break;
        case 'F': DDRF |= mask; PORTF &= ~mask ; break;
        case 'G': DDRG |= mask; PORTG &= ~mask ; break;
        case 'H': DDRH |= mask; PORTH &= ~mask ; break;
        case 'J': DDRJ |= mask; PORTJ &= ~mask ; break;
        case 'K': DDRK |= mask; PORTK &= ~mask ; break;
        case 'L': DDRL |= mask; PORTL &= ~mask ; break;
        default: assert(0);
    }
}


static uint16_t analog_input(uint8_t port, uint8_t pin)
{
    // Enable the ADC.
    ADCSRA |= bit1(ADEN);

    // Use AVCC as reference.
    ADMUX = bit1(REFS0) | pin;

    switch(port) {
        case 'F': ADCSRB &= ~bit1(MUX5) ; break;
        case 'K': ADCSRB |=  bit1(MUX5) ; break;
        default: assert(0);
    }

    // Start the conversion.
    ADCSRA |= bit1(ADSC);

    // Wait for the conversion to complete...
    while (ADCSRA & bit1(ADSC));

    return ADC;
}

static void process_input_message(const uint8_t* const p, const uint8_t l)
{
    assert(l >= 3);

    uint8_t command = p[0];
    uint8_t port = p[1];
    uint8_t pin = p[2];
    assert(pin >= (uint8_t)'0' && pin <= (uint8_t)'7');
    uint8_t pin_n = pin - (uint8_t)'0';

    uint16_t value = 0;

    switch(command) {
        case 'H': output_high(port, pin_n);                  break;
        case 'L': output_low(port, pin_n);                   break;
        case 'U': enable_input_with_pullup(port, pin_n);     break;
        case 'D': enable_input_without_pullup(port, pin_n);  break;
        case 'I': value = read_input(port, pin_n);           break;
        case 'A': value = analog_input(port, pin_n);         break;
    }

    print_c(command);
    print_c(port);
    print_c(pin);
    print_hex16(value);
    print_end_of_line();
}


void main(void) __attribute((noreturn));
void main()
{
    usart_init();

    print_end_of_line();

    sei();

    for(;;) {
        process_input_from_usart();
    }
}



//==============================================================================
// End of file.
//==============================================================================
