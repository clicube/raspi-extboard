#ifndef COMMAND_H
#define COMMAND_H

#include "lib/uart/uart.h"

#define CMD_OK return 0

#define printc(c) uart_putc(c);
#define print_P(msg) uart_puts_P(msg)
#define print(msg) uart_puts(msg)
#define println_P(msg) uart_puts_P(msg "\r\n")
#define println(msg) uart_puts(msg); println_P("")

#define CMD_ERR(code,msg)   print(__func__); print_P(": "); println(msg);   return code
#define CMD_ERR_p(code,msg) print(__func__); print_P(": "); println_p(msg); return code
#define CMD_ERR_P(code,msg) print(__func__); println_P(": " msg);           return code

#endif

