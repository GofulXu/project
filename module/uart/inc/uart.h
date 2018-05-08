#ifndef __UART_H__
#define __UART_H__

#define UART_DEV	"/dev/ttyUSB0"
int start_uart(char *dev, int rate, bool read_bool);
int close_uart(void);
int send_to_uart(char *buf, int size);
#endif /*__UART_H__*/
