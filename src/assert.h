//==============================================================================
// AVR Assert Macro.
// 
// The assert macro passes the current PC (Programm Counter) address to the
// `error` funtion.
// 
// Coypright OC Technology Pty Ltd 2021.
//==============================================================================

#ifndef ASSERT_H_INCLUDED
#define ASSERT_H_INCLUDED


static void error(const uint16_t code, const char* const message)
                  __attribute__ ((noreturn))
                  __attribute__ ((noinline));


#define get_pc() \
({ \
    volatile uint16_t address; \
    asm volatile("rcall next%= \n" \
                 "next%=:      \n" \
                 "pop %B0      \n" \
                 "pop %A0      \n" \
                 : "=w" (address)); \
    address <<= 1U; \
    address; \
})         


#define assert(test, message) \
({ \
    if ((__builtin_expect(!!(!(test)), 0))) { \
        error(get_pc(), message); \
    } \
})



#endif // ASSERT_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
