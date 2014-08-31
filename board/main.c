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
  println_P("\r\n\r\n>>> RasPi-ExtBoard initialized <<<");

  char buf[LINE_MAX_LEN];
  uint8_t ret;
  for(;;)
  {
    print_P(PROMPT);
    getln(buf,LINE_MAX_LEN);
    ret = exec_cmd(buf);
  }

  return 0;
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
    println_P("available command: help reset temp_read ir_scan ir_send");
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
  else if( strcmp_P(cmd_str, PSTR("ir_send")) == 0)
  {
    ret = ir_send(params_str);
  }
  else
  {
    print_P("unknown command: ");
    println(cmd_str);
  }

  return ret;

}
