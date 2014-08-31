#include <avr/wdt.h>
#include "command.h"

void getln(char buf[], uint16_t buflen)
{
  uint16_t c;
  uint16_t buf_idx;

  buf[0] = '\0';
  buf_idx = 0;

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
      if(buf_idx < buflen-1)
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
