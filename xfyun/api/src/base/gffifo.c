#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gffifo.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

int gf_fifo_init(struct cycle_buffer **f, int size)
{
    struct cycle_buffer *fifo = NULL;
    int ret;

    ret = size & (size - 1);
    if (ret)
        return ret;
    fifo = (struct cycle_buffer *)malloc(sizeof(struct cycle_buffer));
    if (!fifo)
        return -1;

    memset(fifo, 0, sizeof(struct cycle_buffer));
    fifo->size = size;
    fifo->in = fifo->out = 0;

    fifo->buf = (unsigned char *)malloc(size);
    if (!fifo->buf) {
        free(fifo);
        return -1;
    }
    else
        memset(fifo->buf, 0, size);

    *f = fifo;

    return 0;
}

int gf_fifo_prepare(struct cycle_buffer *fifo,unsigned char *buf, unsigned int size, pCallback callback)
{
    unsigned int l;
	unsigned int len = size;
    len = min(len, fifo->in - fifo->out);
    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buf, fifo->buf + (fifo->out & (fifo->size - 1)), l);
    memcpy(buf + l, fifo->buf, len - l);

    return callback(buf, size);
}

unsigned int gf_fifo_get(struct cycle_buffer *fifo,unsigned char *buf, unsigned int len)
{
    unsigned int l;
    len = min(len, fifo->in - fifo->out);
    l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buf, fifo->buf + (fifo->out & (fifo->size - 1)), l);
    memcpy(buf + l, fifo->buf, len - l);
    fifo->out += len;
    return len;
}

unsigned int gf_fifo_put(struct cycle_buffer *fifo,unsigned char *buf, unsigned int len)
{
    unsigned int l;
    len = min(len, fifo->size - fifo->in + fifo->out);
    l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buf + (fifo->in & (fifo->size - 1)), buf, l);
    memcpy(fifo->buf, buf + l, len - l);
    fifo->in += len;
    return len;
}

unsigned int gf_fifo_size(struct cycle_buffer *fifo)
{
    return fifo->in - fifo->out;
}
