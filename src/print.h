//==============================================================================
// Debug Message Printing
//==============================================================================

#ifndef PRINT_H_INCLUDED 
#define PRINT_H_INCLUDED 

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define FILE_AND_LINE __FILE__ ":" TOSTRING(__LINE__)


#define debug_ping() print_message(FILE_AND_LINE)
#define debug(m) print_message_str(FILE_AND_LINE, m)
#define debug_int(m, v) print_message_str_int(FILE_AND_LINE, m, v)


static void print(const char* p) {
    while (*p) {print_c((uint8_t)*p++); }
}
static void print_flash(flash const char* p) {
    while (*p) {print_c((uint8_t)*p++); }
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


static void print_message_header(flash const uint8_t* const p_type)
{
    print_flash(p_type);
    print_c(' ');
}


static void print_end_of_line(void)
{
    print_c('\r');
    print_c('\n');
}


static void print_message(flash const uint8_t* const m)
{
    print_flash(m);
    print_end_of_line();
}

static void print_message_str(flash const uint8_t* const p_type,
                              flash const uint8_t* const value)
{
    print_message_header(p_type);
    print_flash(value);
    print_end_of_line();
}


static void print_message_int(flash const uint8_t* const p_type,
                              const uint16_t value)
{
    print_message_header(p_type);
    print_hex16(value);
    print_end_of_line();
}

static void print_message_str_int(flash const uint8_t* const p_type,
                                  flash const uint8_t* const value_a,
                                  const uint16_t value_b)
{
    print_message_header(p_type);
    print_flash(value_a);
    print_c(' ');
    print_hex16(value_b);
    print_end_of_line();
}

static void print_message_2int(flash const uint8_t* const p_type,
                               const uint16_t value_a,
                               const uint16_t value_b)
{
    print_message_header(p_type);
    print_hex16(value_a);
    print_c(' ');
    print_hex16(value_b);
    print_end_of_line();
}


static void print_message_int8(flash const uint8_t* const p_type,
                               const uint8_t value)
{
    print_message_header(p_type);
    print_hex(value);
    print_end_of_line();
}


static void print_message_data(flash const uint8_t* const p_type,
                               const uint8_t* p,
                               uint8_t l)
{
    print_message_header(p_type);
    while(l--) { print_c(*p++); }
    print_end_of_line();
}


static void print_message_hex(flash const uint8_t* const p_type,
                               const uint8_t* p,
                               uint8_t l)
{
    print_message_header(p_type);
    while(l--) {
        print_hex(*p++);
        print_c(' ');
    }
    print_end_of_line();
}



#endif // PRINT_H_INCLUDED 

//==============================================================================
// End of file.
//==============================================================================
