//==============================================================================
// Main CPU Programming and Serial IO Test Harness - Serial IO Module
// 
// Serial Input From R-Pi'
//==============================================================================


// FIFO stores bytes received from AVR USART.
static fifo_t* const p_g_input_fifo = ALLOCATE_FIFO(128);

// On USART RX interrupt, store received byte in FIFO.
ISR(USART0_RX_vect)
{
    assert(fifo_is_not_full(p_g_input_fifo));
    fifo_write(p_g_input_fifo, UDR0);
}


// Buffer for one line of input.
#define INPUT_BUF_LENGTH 32U
#define INPUT_IDLE -1
static int8_t g_input_i = INPUT_IDLE;
static uint8_t g_input_buf[INPUT_BUF_LENGTH];


static void process_input_message(const uint8_t* p_input, uint8_t input_l);

// Read one line at a time from the input FIFO.
// Call `process_input_message()` for each received line.
static void process_input_from_usart(void)
{
    // Do nothing if no bytes have been received.
    if (fifo_is_empty(p_g_input_fifo)) {
        return;
    }

    // Remove one byte from the input FIFO.
    const uint8_t c = fifo_read(p_g_input_fifo);

    if (g_input_i == INPUT_IDLE) {

        // Ignore line delimiters in idle mode.
        if ((c == (uint8_t)'\r')
        ||  (c == (uint8_t)'\n')) {
            return;
        } else {
            g_input_i = 0;
        }
    }

    // At the end of the line: process the message buffer,
    // then return to idle state.
    if (c == (uint8_t)'\r') {
        process_input_message(g_input_buf, g_input_i);
        g_input_i = INPUT_IDLE;
        return;
    }

    // Store the received byte `c` in the input buffer.
    g_input_buf[g_input_i++] = c;

    // Return to idle state if the buffer is full before the end of the line.
    if (g_input_i >= INPUT_BUF_LENGTH) {
        g_input_i = INPUT_IDLE;
    }
}



//==============================================================================
// End of file.
//==============================================================================
