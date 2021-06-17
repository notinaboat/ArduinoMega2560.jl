//==============================================================================
// AVR USART0.
//
// Copyright OC Technology Pty Ltd 2021.
//
// DS40002211A: https://ww1.microchip.com/downloads/en/DeviceDoc/
//              ATmega640-1280-1281-2560-2561-Datasheet-DS40002211A.pdf
//==============================================================================


// UBRR0 value for 38400 bps is 25 (U2X0 = 0, 16 MHz system clock)
// [DS40002211A, Table 22-11, p223]
static void usart_set_38400_bps(void) { UBRR0H = 0U; UBRR0L = 25U; }


// Wake USART0 via Power Reduction Register.
// [DS40002211A, 11.10.2, p55]
static void usart_power_on(void) { PRR0 &= (uint8_t)~bit1(PRUSART0); }


// Enable RX and UDR Interrupts.
// [DS40002211A, 22.10.3, p220]
static void usart_interrupt_enable(void)
{
    UCSR0B = bit4(RXCIE0, UDRIE0, RXEN0, TXEN0);
}


// 8 data bits, no parity, 1 stop bit.
// [DS40002211A, 22.10.4, p221]
static void usart_set_8n1(void) { UCSR0C = bit2(UCSZ01, UCSZ00); }

static void usart_init(void)
{
    usart_set_38400_bps();
    usart_set_8n1();
    usart_power_on();
    usart_interrupt_enable();
}


#ifdef USART0_BLOCKING_RX
static bool usart_rx_is_ready(void) { return UCSR0A & bit1(RXC0); }
static bool usart_rx_is_not_ready(void) { return !usart_rx_is_ready(); }

static uint8_t usart_rx()
{
    while (usart_rx_is_not_ready());
    return UDR0;
}
#endif


static bool usart_tx_is_empty(void) { return UCSR0A & bit1(UDRE0); }
static bool usart_tx_is_not_empty(void) { return !usart_tx_is_empty(); }

static void usart_tx(const uint8_t c)
{
    while (usart_tx_is_not_empty()) {};
    UDR0 = c;
}



//==============================================================================
// End of file.
//==============================================================================
