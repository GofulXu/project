#ifndef __GFSERIAL_H_
#define __GFSERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif


#define GFSERIAL_LOG_DEBUG( format, ...) 	gf_log(LOG_LEVEL_DEBUG, "serial", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSERIAL_LOG_INFO( format, ...) 	gf_log(LOG_LEVEL_INFO, "serial", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFSERIAL_LOG_WARN( format, ...) 	gf_log(LOG_LEVEL_WARN, "serial", __FUNCTION__, __LINE__, format, ##__VA_ARGS__) 
#define GFSERIAL_LOG_ERROR( format, ...) 	gf_log(LOG_LEVEL_ERROR, "serial", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define GFSERIAL_LOG_FATAL( format, ...) 	gf_log(LOG_LEVEL_FATAL, "serial", __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

int32_t gf_serial_open(const char *dev);
void gf_serial_close(int fd);
int32_t gf_serial_read(int fd, unsigned char *buf, int len, int timeout);
int32_t gf_serial_write(int fd, unsigned char *buf, int len, int timeout);
int32_t gf_serial_set_parity(int fd,int speed,int databits,int stopbits,int parity);

#ifdef __cplusplus
}
#endif

#endif
