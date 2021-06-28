//==============================================================================
// AVR TIMER2 Clock.
// 
// Coypright OC Technology Pty Ltd 2021.
// 
// DS40002061B: https://ww1.microchip.com/downloads/en/DeviceDoc/
//                      ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf
//==============================================================================


static volatile uint8_t g_timer2_clock = 0U;
static uint8_t timer2_clock(void) { return g_timer2_clock; }
static void timer2_clock_increment(void) { g_timer2_clock++; }

ISR(TIMER2_COMPA_vect)
{
    timer2_clock_increment();
}

// Clear Timer on Compare mode.
// [DS40002061B, Table 18-8, p164]
static void timer2_ctc_mode(void) { TCCR2A = bit1(WGM21); }

// Clock Select 250 kHz = 16MHz / 64.
// [DS40002061B, 18-9, p165]
static void timer2_clock_250khz(void) { TCCR2B = bit1(CS22); }

// Output Compare 1 kHz = 250 kHz / 250.
// [DS40002061B, 18.11.4, p166]
static void timer2_isr_1khz(void) { OCR2A = 249U; } 

// Clear Output Compare flag and enable interrupt.
// [DS40002061B, 18.11.6, p167]
static void timer2_compa_interrupt_enable(void) { TIMSK2 = bit1(OCIE2A); }


static void timer2_init(void)
{
    timer2_ctc_mode();
    timer2_clock_250khz();
    timer2_isr_1khz();
    timer2_compa_interrupt_enable();
}


static uint8_t ms_clock(void)
{
    return timer2_clock();
}



//==============================================================================
// End of file.
//==============================================================================
