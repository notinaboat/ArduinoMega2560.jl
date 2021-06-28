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
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "assert.h"
#include "bit.h"

#include "avr_gpio.h"
#include "fifo.h"
#define USART0_RX_FIFO_SIZE 255
#define USART0_TX_FIFO_SIZE 255
#include "avr_usart0.h"
#include "avr_usart1.h"
#include "avr_usart2.h"
#include "avr_usart3.h"
#include "avr_timer2.h"
#include "print.h"
#include "linebuf.h"

typedef struct {
    uint8_t mask;
    uint8_t state;
} pin_monitor_t;

static pin_monitor_t pin_monitor_a = {0,0}; 
static pin_monitor_t pin_monitor_b = {0,0}; 
static pin_monitor_t pin_monitor_c = {0,0}; 
static pin_monitor_t pin_monitor_d = {0,0}; 
static pin_monitor_t pin_monitor_e = {0,0}; 
static pin_monitor_t pin_monitor_f = {0,0}; 
static pin_monitor_t pin_monitor_g = {0,0}; 
static pin_monitor_t pin_monitor_h = {0,0}; 
static pin_monitor_t pin_monitor_j = {0,0}; 
static pin_monitor_t pin_monitor_k = {0,0}; 
static pin_monitor_t pin_monitor_l = {0,0}; 

static uint8_t poll_timestamp = 0;


static void poll_port(const uint8_t port, const uint8_t ddr,
                      const uint8_t state, pin_monitor_t* p_pin_monitor)
{
    const uint8_t old_state = p_pin_monitor->state;
    p_pin_monitor->state = state;
    for(uint8_t i = 0 ; i < 8 ; i++) {
        const uint8_t mask = bit1(i);
        if ((mask & p_pin_monitor->mask) != 0) {
            if((state & mask) != (old_state & mask)) {
                poll_timestamp = ms_clock();
                print_c('!');
                print_c((state & mask) ? 'H' : 'L');
                print_c(port);
                print_c('0' + i);
                print_end_of_line();
            }
        }
    }
}


static void poll_ports()
{
    poll_port('A', DDRA, PINA, &pin_monitor_a);
    poll_port('B', DDRB, PINB, &pin_monitor_b);
    poll_port('C', DDRC, PINC, &pin_monitor_c);
    poll_port('D', DDRD, PIND, &pin_monitor_d);
    poll_port('E', DDRE, PINE, &pin_monitor_e);
    poll_port('F', DDRF, PINF, &pin_monitor_f);
    poll_port('G', DDRG, PING, &pin_monitor_g);
    poll_port('H', DDRH, PINH, &pin_monitor_h);
    poll_port('J', DDRJ, PINJ, &pin_monitor_j);
    poll_port('K', DDRK, PINK, &pin_monitor_k);
    poll_port('L', DDRL, PINL, &pin_monitor_l);
}


static void monitor_input(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': pin_monitor_a.mask |= mask; break;
        case 'B': pin_monitor_b.mask |= mask; break;
        case 'C': pin_monitor_c.mask |= mask; break;
        case 'D': pin_monitor_d.mask |= mask; break;
        case 'E': pin_monitor_e.mask |= mask; break;
        case 'F': pin_monitor_f.mask |= mask; break;
        case 'G': pin_monitor_g.mask |= mask; break;
        case 'H': pin_monitor_h.mask |= mask; break;
        case 'J': pin_monitor_j.mask |= mask; break;
        case 'K': pin_monitor_k.mask |= mask; break;
        case 'L': pin_monitor_l.mask |= mask; break;
        default: assert(0, "Bad Monitor Port!");
    }
}


static void unmonitor_input(const uint8_t port, const uint8_t pin)
{
    const uint8_t mask = 1U << (pin & 0x0FU);
    switch(port) {
        case 'A': pin_monitor_a.mask &= ~mask; break;
        case 'B': pin_monitor_b.mask &= ~mask; break;
        case 'C': pin_monitor_c.mask &= ~mask; break;
        case 'D': pin_monitor_d.mask &= ~mask; break;
        case 'E': pin_monitor_e.mask &= ~mask; break;
        case 'F': pin_monitor_f.mask &= ~mask; break;
        case 'G': pin_monitor_g.mask &= ~mask; break;
        case 'H': pin_monitor_h.mask &= ~mask; break;
        case 'J': pin_monitor_j.mask &= ~mask; break;
        case 'K': pin_monitor_k.mask &= ~mask; break;
        case 'L': pin_monitor_l.mask &= ~mask; break;
        default: assert(0, "Bad Monitor Port!");
    }
}


static void forward_usart_rx(const uint8_t prefix,
                             fifo_t* rx_fifo, linebuf_t* linebuf)
{
    linebuf_append(linebuf, rx_fifo);
    if (linebuf_is_ready(linebuf)) {
        print_c(prefix);
        print_n(linebuf->line, linebuf->l);
        print_end_of_line();
        linebuf_reset(linebuf);
    }
}


static void forward_usart_tx(const uint8_t port,
                             const uint8_t* const p, const uint8_t l)
{
    assert(l >= 1, "Empty Serial Message!");
    assert(port >= (uint8_t)'1' && port <= (uint8_t)'3', "Bad USART!");
    for (uint8_t i = 0 ; i < l ; i++) {
        switch(port) {
            case '1': usart1_tx(p[i]); break;
            case '2': usart2_tx(p[i]); break;
            case '3': usart3_tx(p[i]); break;
        }
    }
}


static void process_command(const uint8_t* const p, const uint8_t l)
{
    // Reset.
    if (l == 1 && p[0] == 'Z') {
        wdt_enable(0);
        for(;;);
    }

    // Serial.
    if (p[0] >= (uint8_t)'1' && p[0] <= (uint8_t)'3') {
        assert(l >= 2, "Short Serial Message!");
        forward_usart_tx(p[0], p+1, l-1);
        forward_usart_tx(p[0], "\r\n", 2);
        print_c('>');
        print_n(p, l);
        print_end_of_line();
        return;
    }

    // GPIO.
    assert(l >= 3, "Short Command!");
    uint8_t command = p[0];
    uint8_t port = p[1];
    uint8_t pin = p[2];
    assert(pin >= (uint8_t)'0' && pin <= (uint8_t)'7', "Bad GPIO Pin!");
    uint8_t pin_n = pin - (uint8_t)'0';

    uint16_t value = 0;

    switch(command) {
        case 'H': output_high(port, pin_n);                  break;
        case 'L': output_low(port, pin_n);                   break;
        case 'U': enable_input_with_pullup(port, pin_n);     break;
        case 'D': enable_input_without_pullup(port, pin_n);  break;
        case 'I': value = read_input(port, pin_n);           break;
        case 'M': monitor_input(port, pin_n);                break;
        case 'N': unmonitor_input(port, pin_n);              break;
        case 'A': value = analog_input(port, pin_n);         break;
    }

    print_c('>');
    print_c(command);
    print_c(port);
    print_c(pin);
    if (command == 'I' || command == 'A') {
        print_hex16(value);
    }
    print_end_of_line();
}

static linebuf_t* const usart0_linebuf = ALLOCATE_LINEBUF(32);
static linebuf_t* const usart1_linebuf = ALLOCATE_LINEBUF(32);
static linebuf_t* const usart2_linebuf = ALLOCATE_LINEBUF(32);
static linebuf_t* const usart3_linebuf = ALLOCATE_LINEBUF(32);


void main(void) __attribute((noreturn));
void main()
{
    usart0_init();
    usart1_init();
    usart2_init();
    usart3_init();

    timer2_init();

    sei();

    print_end_of_line();
    print_c('>');
    print_c('Z');
    print_end_of_line();

    for(;;) {

        linebuf_append(usart0_linebuf, p_g_usart0_rx_fifo);
        if (linebuf_is_ready(usart0_linebuf)) {
            process_command(usart0_linebuf->line, usart0_linebuf->l);
            linebuf_reset(usart0_linebuf);
        }

        forward_usart_rx('1', p_g_usart1_rx_fifo, usart1_linebuf);
        forward_usart_rx('2', p_g_usart2_rx_fifo, usart2_linebuf);
        forward_usart_rx('3', p_g_usart3_rx_fifo, usart3_linebuf);

        uint8_t dt = ms_clock() - poll_timestamp;
        if (dt > 20) {
            poll_ports();
        }
    }
}



//==============================================================================
// End of file.
//==============================================================================
