//==============================================================================
// AVR USART2.
//
// Copyright OC Technology Pty Ltd 2021.
//
// DS40002211A: https://ww1.microchip.com/downloads/en/DeviceDoc/
//              ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
//==============================================================================

#ifndef AVR_USART2_H_INCLUDED
#define AVR_USART2_H_INCLUDED


/* Config */

// UBRRn value for 38400 bps is 25 (U2Xn = 0, 16 MHz system clock)
// [DS40002211A, Table 22-11, p223]
static void usart2_set_16_mhz_38400_bps(void) { UBRR2H = 0U; UBRR2L = 25U; }


// Wake USART2 via Power Reduction Register.
// [DS40002211A, 11.10.3, p56]
static void usart2_power_on(void) { PRR1 &= (uint8_t)~bit1(PRUSART2); }


// Enable RX/TX and RX Interrupt.
// [DS40002211A, 22.10.3, p220]
static void usart2_enable(void) { UCSR2B = bit3(RXCIE2, RXEN2, TXEN2); }


// 8 data bits, no parity, 1 stop bit.
// [DS40002211A, 22.10.4, p221]
static void usart2_set_8n1(void) { UCSR2C = bit2(UCSZ21, UCSZ20); }


static void usart2_init(void)
{
    usart2_set_16_mhz_38400_bps();
    usart2_set_8n1();
    usart2_power_on();
    usart2_enable();
}



/* RX */

#ifndef USART2_RX_FIFO_SIZE
#define USART2_RX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart2_rx_fifo = ALLOCATE_FIFO(USART2_RX_FIFO_SIZE);


// On USART RX interrupt, store received byte in FIFO.
ISR(USART2_RX_vect)
{
    if (fifo_is_full(p_g_usart2_rx_fifo)) {
        fifo_read(p_g_usart2_rx_fifo);
    }
    fifo_write(p_g_usart2_rx_fifo, UDR2);
}



/* TX */

// Enable TX Interrupt.
static void usart2_tx_interrupt_enable(void) { UCSR2B |= bit1(UDRIE2); }


// Disable TX Interrupt.
static void usart2_tx_interrupt_disable(void) { UCSR2B &= ~bit1(UDRIE2); }


#ifndef USART2_TX_FIFO_SIZE
#define USART2_TX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart2_tx_fifo = ALLOCATE_FIFO(USART2_TX_FIFO_SIZE);


// On USART TX interrupt, send byte from FIFO.
ISR(USART2_UDRE_vect)
{
    if (fifo_is_not_empty(p_g_usart2_tx_fifo)) {
        UDR2 = fifo_read(p_g_usart2_tx_fifo);
    } else {
        usart2_tx_interrupt_disable();
    }
}


static bool usart2_tx_is_empty(void) { return UCSR2A & bit1(UDRE2); }
static bool usart2_tx_is_not_empty(void) { return !usart2_tx_is_empty(); }


// Put byte into TX FIFO, ensure that TX interrupt is enabled.
static void usart2_tx(const uint8_t c)
{
    fifo_write(p_g_usart2_tx_fifo, c);

    // If interrupts are globally disabled send bytes directly to UDR2.
    if ((SREG & bit1(SREG_I)) == 0) {
        while (fifo_is_not_empty(p_g_usart2_tx_fifo)) {
            while (usart2_tx_is_not_empty()) {};
            UDR2 = fifo_read(p_g_usart2_tx_fifo);
        }
    } else {
        usart2_tx_interrupt_enable();
    }
}



#endif // AVR_USART2_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
