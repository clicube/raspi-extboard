#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lib/uart/uart.h"
#include "irctrl.h"
#include "command.h"

uint8_t start_scan(uint16_t period);
static inline void stop_scan(void);

struct _scan_state_t
{
  uint8_t end_flag;
  uint8_t prev_state;
  uint8_t cont_count;
  uint8_t data_count;
  uint8_t data[32];
};
typedef struct _scan_state_t scan_state_t;

scan_state_t* scan_state;

void ir_init()
{
  /* Pin initialization */
  /* set PD7, Hi-Z */
  DDRD |= _BV(PD7);
  PORTD &= ~_BV(PD7);
}

uint8_t ir_scan(char* params_str)
{
  uint8_t ret;

  uint16_t period; /* period in us */
  period = atoi(params_str);

  if(period <= 0)
  {
    CMD_ERR_P(IRCTRL_ERR_PARAM_INVALID, "invalid param");
  }
  else if(period > 5000)
  {
    CMD_ERR_P(IRCTRL_ERR_PARAM_TOO_LARGE,"param too large");
  }

  //scan_state = malloc(sizeof(scan_state));
  scan_state_t ss;
  scan_state = &ss;
  ret = start_scan(period);
  if(ret == 0)
  {
    while(scan_state->end_flag==0); // wait for scan
  }
  //free(scan_state);
  scan_state = NULL;

  switch(ret)
  {
    case IRCTRL_ERR_PARAM_TOO_SMALL:
      CMD_ERR_P(ret,"param too small");
    case IRCTRL_ERR_PARAM_TOO_LARGE:
      CMD_ERR_P(ret,"param too large");
  }

  CMD_OK;
}

uint8_t start_scan(uint16_t period)
{
  /* calculation OCR0A ( TOP value for timer0 )
   *         and CS0n  ( N: division raito for timer0 )
   *
   * period = 1/freq
   * freq = F_CPU/(N*(TOP+1))
   * TOP = period*F_CPU/N-1
   * threathold = 256*N+N = 257*N
   */

  uint32_t tmp;
  uint8_t cs0n;
  uint8_t top;

  tmp = period * F_CPU / 1000000;

  if( tmp == 0 )
  {
    return IRCTRL_ERR_PARAM_TOO_SMALL;
  }
  else if( tmp < 257*1 )
  {
    // N = 1
    cs0n = 0b001;
    top = tmp/1-1;
  }
  else if( tmp < 257*8 )
  {
    // N = 8
    cs0n = 0b010;
    top = tmp/8-1;
  }
  else if( tmp < 257*64 )
  {
    // N = 64
    cs0n = 0b011;
    top = tmp/64-1;
  }
  else if( tmp < 257UL*256 )
  {
    // N = 256
    cs0n = 0b100;
    top = tmp/256-1;
  }
  else if( tmp < 257UL*1024 )
  {
    // N = 1024
    cs0n = 0b101;
    top = tmp/1024-1;
  }
  else
  {
    return IRCTRL_ERR_PARAM_TOO_LARGE;
  }

  /* initialize vars */
  scan_state->end_flag = 0;
  scan_state->prev_state = 0xff;
  scan_state->cont_count = 0;
  scan_state->data_count = 0;

  /* set register
   * Timer mode: CTC (WGM0n = 0b010)
   * Interrupt Enable (comp match) : OCIE0A = 1
   *
   * ==> start timer
   */
  TCCR0A = 0b00000010;
  TCCR0B = 0x00 | cs0n;
  OCR0A = top;
  TIMSK0 = 0b00000010;

  return 0;
}

void stop_scan()
{
  TIMSK0 = 0x00; // stop interruption
  scan_state->end_flag = 1;
}

// called every timer0A comp match */
ISR(TIMER0_COMPA_vect)
{
  uint8_t input;
  input = (PIND & _BV(PD7))>>PD7;

  // wait for first input
  if(scan_state->prev_state == 0xff && input == 0)
  {
    return;
  }

  // detect end of signal
  if(input != scan_state->prev_state)
  {
    scan_state->cont_count = 0;
  }
  else
  {
    scan_state->cont_count++;
    if(scan_state->cont_count == 0) // overflow
    {
      stop_scan();
      return;
    }
  }

  scan_state->data_count++;

  uint8_t pos1, pos2;
  pos1 = scan_state->data_count / 8;
  pos2 = scan_state->data_count % 8;

  uint8_t data;
  data = scan_state->data[pos1];

  scan_state->data[pos1] = ( data & ~(1<<pos2) ) | (input<<pos2);

  return;

}
