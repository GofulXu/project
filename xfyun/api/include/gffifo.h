#ifndef __fifo_h
#define __fifo_h

#ifdef __cplusplus
extern "C"{
#endif

#define FIFO 1

enum
{
	DATA_INVALID,
	DATA_VALID,
};

struct cycle_buffer {
    unsigned char *buf;
    unsigned int size;
    unsigned int in;
    unsigned int out;
};

typedef int (*pCallback)(unsigned char *buf, unsigned int len);

int gf_fifo_init(struct cycle_buffer **fifo, int size);

int gf_fifo_prepare(struct cycle_buffer *fifo,unsigned char *buf, unsigned int len, pCallback callback);

unsigned int gf_fifo_get(struct cycle_buffer *fifo,unsigned char *buf, unsigned int len);

unsigned int gf_fifo_put(struct cycle_buffer *fifo,unsigned char *buf, unsigned int len);

unsigned int gf_fifo_size(struct cycle_buffer *fifo);

#ifdef __cplusplus
}
#endif

#endif
