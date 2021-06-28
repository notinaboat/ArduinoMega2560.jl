//==============================================================================
// AVR USART1.
//
// Copyright OC Technology Pty Ltd 2021.
//
// DS40002211A: https://ww1.microchip.com/downloads/en/DeviceDoc/
//              ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
//==============================================================================

#ifndef AVR_USART1_H_INCLUDED
#define AVR_USART1_H_INCLUDED


/* Config */

// UBRRn value for 38400 bps is 25 (U2Xn = 0, 16 MHz system clock)
// [DS40002211A, Table 22-11, p223]
static void usart1_set_16_mhz_38400_bps(void) { UBRR1H = 0U; UBRR1L = 25U; }


// Wake USART1 via Power Reduction Register.
// [DS40002211A, 11.10.3, p56]
static void usart1_power_on(void) { PRR1 &= (uint8_t)~bit1(PRUSART1); }


// Enable RX/TX and RX Interrupt.
// [DS40002211A, 22.10.3, p220]
static void usart1_enable(void) { UCSR1B = bit3(RXCIE1, RXEN1, TXEN1); }


// 8 data bits, no parity, 1 stop bit.
// [DS40002211A, 22.10.4, p221]
static void usart1_set_8n1(void) { UCSR1C = bit2(UCSZ11, UCSZ10); }


static void usart1_init(void)
{
    usart1_set_16_mhz_38400_bps();
    usart1_set_8n1();
    usart1_power_on();
    usart1_enable();
}



/* RX */

#ifndef USART1_RX_FIFO_SIZE
#define USART1_RX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart1_rx_fifo = ALLOCATE_FIFO(USART1_RX_FIFO_SIZE);


// On USART RX interrupt, store received byte in FIFO.
ISR(USART1_RX_vect)
{
    if (fifo_is_full(p_g_usart1_rx_fifo)) {
        fifo_read(p_g_usart1_rx_fifo);
    }
    fifo_write(p_g_usart1_rx_fifo, UDR1);
}



/* TX */

// Enable TX Interrupt.
static void usart1_tx_interrupt_enable(void) { UCSR1B |= bit1(UDRIE1); }


// Disable TX Interrupt.
static void usart1_tx_interrupt_disable(void) { UCSR1B &= ~bit1(UDRIE1); }


#ifndef USART1_TX_FIFO_SIZE
#define USART1_TX_FIFO_SIZE 32
#endif

static fifo_t* const p_g_usart1_tx_fifo = ALLOCATE_FIFO(USART1_TX_FIFO_SIZE);


// On USART TX interrupt, send byte from FIFO.
ISR(USART1_UDRE_vect)
{
    if (fifo_is_not_empty(p_g_usart1_tx_fifo)) {
        UDR1 = fifo_read(p_g_usart1_tx_fifo);
    } else {
        usart1_tx_interrupt_disable();
    }
}


static bool usart1_tx_is_empty(void) { return UCSR1A & bit1(UDRE1); }
static bool usart1_tx_is_not_empty(void) { return !usart1_tx_is_empty(); }


// Put byte into TX FIFO, ensure that TX interrupt is enabled.
static void usart1_tx(const uint8_t c)
{
    fifo_write(p_g_usart1_tx_fifo, c);

    // If interrupts are globally disabled send bytes directly to UDR1.
    if ((SREG & bit1(SREG_I)) == 0) {
        while (fifo_is_not_empty(p_g_usart1_tx_fifo)) {
            while (usart1_tx_is_not_empty()) {};
            UDR1 = fifo_read(p_g_usart1_tx_fifo);
        }
    } else {
        usart1_tx_interrupt_enable();
    }
}



#endif // AVR_USART1_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
