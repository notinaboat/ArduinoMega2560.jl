//==============================================================================
// Print to USART.
// 
// Copyright OC Technology Pty Ltd 2021.
//==============================================================================

#ifndef PRINT_H_INCLUDED
#define PRINT_H_INCLUDED


#ifndef PRINT_C
#define PRINT_C usart0_tx
#endif


static void print_c(const uint8_t c) { PRINT_C(c); }


static void print(const char* p) { while (*p) {print_c((uint8_t)*p++); } }


void print_n(const uint8_t* const p, const uint8_t n)
{
    for (uint8_t i = 0 ; i < n ; i++) {
        print_c(p[i]);
    }
}


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


static void print_end_of_line(void)
{
    print_c('\r');
    print_c('\n');
}


static void error(const uint16_t code, const char* const message)
{
    cli();

    print_end_of_line();
    print("ERROR ");
    print_hex16(code);
    print_c(' ');
    print(message);
    print_end_of_line();
    wdt_enable(0);
    for(;;);
}



#endif // PRINT_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
