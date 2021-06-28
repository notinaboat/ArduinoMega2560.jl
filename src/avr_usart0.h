//==============================================================================
// AVR USART0.
//
// Copyright OC Technology Pty Ltd 2021.
//
// DS40002211A: https://ww1.microchip.com/downloads/en/DeviceDoc/
//              ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
//==============================================================================

#ifndef AVR_USART0_H_INCLUDED
#define AVR_USART0_H_INCLUDED

#ifndef UBRR0H
#define UBRR0H UBRRH
#define UBRR0L UBRRL
#define UCSR0B UCSRB
#define UCSR0C UCSRC
#define RXCIE0 RXCIE
#define RXEN0 RXEN
#define TXEN0 TXEN
#define UCSZ00 UCSZ0
#define UCSZ01 UCSZ1
#define USART0_RX_vect USART_RXC_vect
#define USART0_UDRE_vect USART_UDRE_vect
#define UDRIE0 UDRIE
#define UCSR0A UCSRA
#define UDRE0 UDRE
#define UDR0 UDR
#endif


/* Config */

// UBRRn value for 38400 bps is 25 (U2X0 = 0, 16 MHz system clock)
// [DS40002211A, Table 22-11, p223]
static void usart0_set_16_mhz_38400_bps(void) { UBRR0H = 0U; UBRR0L = 25U; }

// UBRRn value for 38400 bps is 23 (U2X0 = 0, 14.7456 MHz system clock)
// [DS40002211A, Table 22-11, p223]
void usart0_set_14_mhz_38400_bps(void) { UBRR0H = 0U; UBRR0L = 23U; }


// Wake USART0 via Power Reduction Register.
// [DS40002211A, 11.10.3, p56]
static void usart0_power_on(void) {
    #ifdef PRR0
        PRR0 &= (uint8_t)~bit1(PRUSART0);
    #endif
}


// Enable RX/TX and RX Interrupt.
// [DS40002211A, 22.10.3, p220]
static void usart0_enable(void) { UCSR0B = bit3(RXCIE0, RXEN0, TXEN0); }


// 8 data bits, no parity, 1 stop bit.
// [DS40002211A, 22.10.4, p221]
static void usart0_set_8n1(void) { UCSR0C = bit2(UCSZ01, UCSZ00); }


static void usart0_init(void)
{
    usart0_set_16_mhz_38400_bps();
    usart0_set_8n1();
    usart0_power_on();
    usart0_enable();
}



/* RX */

#ifndef USART0_RX_FIFO_SIZE
#define USART0_RX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart0_rx_fifo = ALLOCATE_FIFO(USART0_RX_FIFO_SIZE);


// On USART RX interrupt, store received byte in FIFO.
ISR(USART0_RX_vect)
{
    if (fifo_is_full(p_g_usart0_rx_fifo)) {
        fifo_read(p_g_usart0_rx_fifo);
    }
    fifo_write(p_g_usart0_rx_fifo, UDR0);
}



/* TX */

// Enable TX Interrupt.
static void usart0_tx_interrupt_enable(void) { UCSR0B |= bit1(UDRIE0); }


// Disable TX Interrupt.
static void usart0_tx_interrupt_disable(void) { UCSR0B &= ~bit1(UDRIE0); }


#ifndef USART0_TX_FIFO_SIZE
#define USART0_TX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart0_tx_fifo = ALLOCATE_FIFO(USART0_TX_FIFO_SIZE);


// On USART TX interrupt, send byte from FIFO.
ISR(USART0_UDRE_vect)
{
    if (fifo_is_not_empty(p_g_usart0_tx_fifo)) {
        UDR0 = fifo_read(p_g_usart0_tx_fifo);
    } else {
        usart0_tx_interrupt_disable();
    }
}


static bool usart0_tx_is_empty(void) { return UCSR0A & bit1(UDRE0); }
static bool usart0_tx_is_not_empty(void) { return !usart0_tx_is_empty(); }


// Put byte into TX FIFO, ensure that TX interrupt is enabled.
static void usart0_tx(const uint8_t c)
{
    fifo_write(p_g_usart0_tx_fifo, c);

    // If interrupts are globally disabled send bytes directly to UDR0.
    if ((SREG & bit1(SREG_I)) == 0) {
        while (fifo_is_not_empty(p_g_usart0_tx_fifo)) {
            while (usart0_tx_is_not_empty()) {};
            UDR0 = fifo_read(p_g_usart0_tx_fifo);
        }
    } else {
        usart0_tx_interrupt_enable();
    }
}



#endif // AVR_USART0_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
