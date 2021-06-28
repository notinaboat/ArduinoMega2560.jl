//==============================================================================
// AVR USART3.
//
// Copyright OC Technology Pty Ltd 2021.
//
// DS40002211A: https://ww1.microchip.com/downloads/en/DeviceDoc/
//              ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
//==============================================================================

#ifndef AVR_USART3_H_INCLUDED
#define AVR_USART3_H_INCLUDED


/* Config */

// UBRRn value for 38400 bps is 25 (U2Xn = 0, 16 MHz system clock)
// [DS40002211A, Table 22-11, p223]
static void usart3_set_16_mhz_38400_bps(void) { UBRR3H = 0U; UBRR3L = 25U; }


// Wake USART3 via Power Reduction Register.
// [DS40002211A, 11.10.3, p56]
static void usart3_power_on(void) { PRR1 &= (uint8_t)~bit1(PRUSART3); }


// Enable RX/TX and RX Interrupt.
// [DS40002211A, 22.10.3, p220]
static void usart3_enable(void) { UCSR3B = bit3(RXCIE3, RXEN3, TXEN3); }


// 8 data bits, no parity, 1 stop bit.
// [DS40002211A, 22.10.4, p221]
static void usart3_set_8n1(void) { UCSR3C = bit2(UCSZ31, UCSZ30); }


static void usart3_init(void)
{
    usart3_set_16_mhz_38400_bps();
    usart3_set_8n1();
    usart3_power_on();
    usart3_enable();
}



/* RX */

#ifndef USART3_RX_FIFO_SIZE
#define USART3_RX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart3_rx_fifo = ALLOCATE_FIFO(USART3_RX_FIFO_SIZE);


// On USART RX interrupt, store received byte in FIFO.
ISR(USART3_RX_vect)
{
    if (fifo_is_full(p_g_usart3_rx_fifo)) {
        fifo_read(p_g_usart3_rx_fifo);
    }
    fifo_write(p_g_usart3_rx_fifo, UDR3);
}



/* TX */

// Enable TX Interrupt.
static void usart3_tx_interrupt_enable(void) { UCSR3B |= bit1(UDRIE3); }


// Disable TX Interrupt.
static void usart3_tx_interrupt_disable(void) { UCSR3B &= ~bit1(UDRIE3); }


#ifndef USART3_TX_FIFO_SIZE
#define USART3_TX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart3_tx_fifo = ALLOCATE_FIFO(USART3_TX_FIFO_SIZE);


// On USART TX interrupt, send byte from FIFO.
ISR(USART3_UDRE_vect)
{
    if (fifo_is_not_empty(p_g_usart3_tx_fifo)) {
        UDR3 = fifo_read(p_g_usart3_tx_fifo);
    } else {
        usart3_tx_interrupt_disable();
    }
}


static bool usart3_tx_is_empty(void) { return UCSR3A & bit1(UDRE3); }
static bool usart3_tx_is_not_empty(void) { return !usart3_tx_is_empty(); }


// Put byte into TX FIFO, ensure that TX interrupt is enabled.
static void usart3_tx(const uint8_t c)
{
    fifo_write(p_g_usart3_tx_fifo, c);

    // If interrupts are globally disabled send bytes directly to UDR3.
    if ((SREG & bit1(SREG_I)) == 0) {
        while (fifo_is_not_empty(p_g_usart3_tx_fifo)) {
            while (usart3_tx_is_not_empty()) {};
            UDR3 = fifo_read(p_g_usart3_tx_fifo);
        }
    } else {
        usart3_tx_interrupt_enable();
    }
}



#endif // AVR_USART3_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
