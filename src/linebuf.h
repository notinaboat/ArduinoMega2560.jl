//==============================================================================
// Input Line buffer.
// Reads bytes from a FIFO one line at a time.
// 
// Copyright OC Technology Pty Ltd 2021.
//==============================================================================

#ifndef LINEBUF_H_INCLUDED
#define LINEBUF_H_INCLUDED


typedef struct {
    const uint8_t size;
    uint8_t ready;
    uint8_t l;
    uint8_t line[];
} linebuf_t;


#define ALLOCATE_LINEBUF(static_size) ((linebuf_t*) \
        &(struct { linebuf_t linebuf; uint8_t buf[(static_size)]; }) \
        {.linebuf = {.size = (static_size)}})


static void linebuf_reset(linebuf_t* linebuf)
{
    linebuf->ready = 0;
    linebuf->l = 0;
}


static bool linebuf_is_ready(linebuf_t* linebuf) { return linebuf->ready; }


static void linebuf_append(linebuf_t* linebuf, fifo_t* fifo)
{
    assert(!linebuf_is_ready(linebuf), "Linebuf Not Reset!");

    while (fifo_is_not_empty(fifo)) {

        const uint8_t c = fifo_read(fifo);

        // Ignore leading line delimiters.
        if ((linebuf->l == 0) && ((c == '\0') || (c == '\r') || (c == '\n'))) {
            continue;
        }

        // Terminate at end of line.
        if (c == '\r' || c == '\n') {
            linebuf->ready = 1;
            return;
        }

        // Store the received byte `c` in the buffer.
        linebuf->line[linebuf->l++] = c;

        // Check for overrun.
        assert(linebuf->l < linebuf->size, "Linebuf Overrun!");
    }
}



#endif // LINEBUF_H_INCLUDED

//==============================================================================
// End of file.
//==============================================================================
