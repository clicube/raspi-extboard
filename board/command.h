#ifndef COMMAND_H
#define COMMAND_H

#include "lib/uart/uart.h"

#define CMD_OK return 0

#define _puts_msg(func) uart_puts(__func__); uart_puts_P(": "); func; uart_puts_P("\r\n")
#define CMD_ERR(code,msg)   _puts_msg(uart_puts(msg));   return code
#define CMD_ERR_p(code,msg) _puts_msg(uart_puts_p(msg)); return code
#define CMD_ERR_P(code,msg) _puts_msg(uart_puts_P(msg)); return code

#endif

