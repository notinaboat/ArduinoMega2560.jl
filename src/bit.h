//==============================================================================
// Bit Utilities
//
// Copyright OC Technology Pty Ltd 2021
//==============================================================================

#ifndef BIT_H_INCLUDED
#define BIT_H_INCLUDED


static uint8_t bit1(const uint8_t i)
{
    return (uint8_t)(1U << i);
}

static uint8_t bit2(const uint8_t i,
                    const uint8_t j)
{
     return bit1(i) | bit1(j);
}

static uint8_t bit3(const uint8_t i,
                    const uint8_t j,
                    const uint8_t k)
{
    return bit1(i) | bit2(j, k);
}

static uint8_t bit4(const uint8_t i,
                    const uint8_t j,
                    const uint8_t k,
                    const uint8_t l)
{
    return bit1(i) | bit3(j, k, l);
}



#endif // BIT_H_INCLUDED

//==============================================================================
// End of file
//==============================================================================
