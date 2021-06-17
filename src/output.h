//==============================================================================
// Main CPU Programming and Serial IO Test Harness - Serial IO Module
// 
// Serial Output To R-Pi'
//==============================================================================


static fifo_t* const p_g_output_fifo = ALLOCATE_FIFO(64);

ISR(USART0_UDRE_vect)
{
    if (fifo_is_not_empty(p_g_output_fifo)) {
        UDR0 = fifo_read(p_g_output_fifo);
    }
}

static bool interrupts_are_disabled(void)
{
    return (SREG & bit1(SREG_I)) == 0;
}

static void process_output(void)
{
    if (interrupts_are_disabled()) {
        while (fifo_is_not_empty(p_g_output_fifo)) {
            usart_tx(fifo_read(p_g_output_fifo));
        }
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if ((usart_tx_is_empty()) && (fifo_is_not_empty(p_g_output_fifo))) {
            usart_tx(fifo_read(p_g_output_fifo));
        }
    }
}


static void print_c(const uint8_t c) { fifo_write(p_g_output_fifo, c); }

static void print(const char* p) { while (*p) {print_c((uint8_t)*p++); } }


static void print_hex(const uint8_t x)
{
    uint8_t n = x >> 4U;
    n += (n > 9U) ? ((uint8_t)'A' - 10U) : '0';
    print_c(n);

    n = x & 0x0FU;
    n += (n > 9U) ? ((uint8_t)'A' - 10U) : '0';
    print_c(n);
}


static void print_hex16(const uint16_t x)
{
    print_hex(x >> 8U);
    print_hex(x & 0xFFU);
}


static void print_message_header(const uint8_t* const p_type)
{
    print(p_type);
    print_c(' ');
}


static void print_end_of_line(void)
{
    print_c('\r');
    print_c('\n');
    process_output();
}


__attribute__ ((noinline))
static void print_message_int(const uint8_t* const p_type,
                              const uint16_t value)
{
    print_message_header(p_type);
    print_hex16(value);
    print_end_of_line();
}


static void error(const uint16_t code)
{
    cli();

    print_end_of_line();
    for(;;) {
        print_message_int("ERROR", code);
        _delay_ms(10000);
    }
}



//==============================================================================
// End of file.
//==============================================================================
