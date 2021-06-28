//==============================================================================
// FIFO buffer.
// 
// Copyright OC Technology Pty Ltd 2021.
//==============================================================================

#ifndef FIFO_H_INCLUDED
#define FIFO_H_INCLUDED


typedef struct fifo
{
    volatile uint8_t in;
    volatile uint8_t out;
    const uint8_t size;
    uint8_t buf[];
} fifo_t;


#define ALLOCATE_FIFO(static_size) ((fifo_t*) \
        &(struct { fifo_t fifo; uint8_t buf[(static_size)]; }) \
        {.fifo = {.size = (static_size)}})


static uint8_t next_fifo_i(const fifo_t* const p, uint8_t i)
{
    return (uint8_t)(++i % p->size);
}


static bool fifo_is_empty(const fifo_t* const p)
{
    return p->in == p->out;
}


static bool fifo_is_full(const fifo_t* const p)
{
    return next_fifo_i(p, p->in) == p->out;
}


static bool fifo_is_not_empty(const fifo_t* const p)
{
    return !fifo_is_empty(p);
}

bool fifo_is_not_full(const fifo_t* const p)
{
    return !fifo_is_full(p);
}


static uint8_t fifo_read(fifo_t* const p)
{
    while (fifo_is_empty(p)) {
        // Wait.
    }

    /* local scope */ {
        const uint8_t i = p->out;
        const uint8_t c = p->buf[i];
        p->out = next_fifo_i(p, i);
        return c;
    }
}


static void fifo_write(fifo_t* const p, const uint8_t c)
{
    while (fifo_is_full(p)) {
        // Wait.
    }

    /* local scope */ {
        const uint8_t i = p->in;
        p->buf[i] = c;
        p->in = next_fifo_i(p, i);
    }
}



#endif // FIFO_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
