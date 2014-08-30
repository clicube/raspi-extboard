#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lib/uart/uart.h"
#include "lib/i2cmaster/i2cmaster.h"
#include "irctrl.h"
#include "tempctrl.h"
#include "command.h"


#define UART_BAUD_RATE 9600
#define LINE_MAX_LEN 32
#define PROMPT "RasPi-ExtBoard> "

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
int main(void);
void wait_for_cmd(char*);
static inline uint8_t exec_cmd(char*);

void wdt_init(void)
{
    MCUSR = 0;
    //wdt_disable();
    wdt_enable(WDTO_4S);
    return;
}

int main(void)
{
  uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU)); 
  i2c_init();
  ir_init();
  temp_init();
  sei();

  wdt_reset();
  uart_puts_P("\r\n\r\n>>> RasPi-ExtBoard initialized <<<\r\n");

  char buf[LINE_MAX_LEN];
  uint8_t ret;
  for(;;)
  {
    wait_for_cmd(buf);
    ret = exec_cmd(buf);
  }

  return 0;
}

void wait_for_cmd(char* buf)
{
  uint16_t c;
  uint8_t buf_idx;

  buf[0] = '\0';
  buf_idx = 0;
  uart_puts_P(PROMPT);

  for(;;)
  {
    wdt_reset();
    c = uart_getc();
    if(c & UART_NO_DATA){ continue; }

    if( c == '\n' || c == '\r' )
    {
      uart_puts_p(CRLF_P);
      buf[buf_idx] = '\0';
      return;
    }
    else if( c == '\b' || c == '\x7f')
    {
      if(buf_idx > 0)
      {
        buf[buf_idx] = '\0';
        buf_idx--;
        uart_puts_P("\b \b");
      }
      else
      {
        uart_putc('\a');
      }

    }
    else if( 32 <= c && c <=126 )
    {
      if(buf_idx < LINE_MAX_LEN-1)
      {
        uart_putc((char)c);
        buf[buf_idx] = (char)c;
        buf_idx++;
      }
      else
      {
        uart_putc('\a');
      }
    }
  }
}

uint8_t exec_cmd(char* str)
{
  uint8_t ret = 255;

  /* skip spaces */
  while( *str == ' ' ){ str++; }

  /* return if no cmd */
  if( *str == '\0' ){ return 0; }

  char* cmd_str;
  char* params_str;

  cmd_str = strtok_P(str,PSTR(" "));
  params_str = strtok_P(NULL, PSTR(""));

  /* skip spaces */
  while( *params_str == ' ' ){ params_str++; }

  if( strcmp_P(cmd_str, PSTR("help")) == 0)
  {
    uart_puts_P("available command: help reset temp_read ir_recv\r\n");
    ret = 0;
  }
  else if( strcmp_P(cmd_str, PSTR("reset")) == 0)
  {
    wdt_enable(WDTO_15MS);
    cli();
    for(;;);
  }
  else if( strcmp_P(cmd_str, PSTR("temp_read")) == 0)
  {
    ret = temp_read(params_str);
  }
  else if( strcmp_P(cmd_str, PSTR("ir_scan")) == 0)
  {
    ret = ir_scan(params_str);
  }
  else
  {
    uart_puts_P("unknown command: ");
    uart_puts(cmd_str);
    uart_puts_p(CRLF_P);
  }

  return ret;

}
