#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lib/uart/uart.h"
#include "irctrl.h"
#include "command.h"

#define IRCTRL_STATE_NOP 0
#define IRCTRL_STATE_SCAN 1
#define IRCTRL_STATE_SEND 2

uint8_t start_timer(uint16_t period);
static inline void stop_timer();
static inline uint8_t start_scan(uint16_t period);
static inline void stop_scan(void);
static inline void isr_scan(void);
static inline void isr_send(void);

struct _scan_state_t
{
  volatile uint8_t end_flag;
  uint16_t cont_count;
  uint16_t data_count;
  uint8_t data[IRCTRL_BUF_LEN];
};
typedef struct _scan_state_t scan_state_t;

uint8_t irctrl_state;
scan_state_t* scan_state;


void ir_init()
{
  /* Pin initialization */
  /* set PD7 to input, Hi-Z */
  DDRD  &= ~_BV(PIN7);
  PORTD &= ~_BV(PIN7);

  /* set PB3 to output, Low */
  DDRB  |= _BV(PIN3);
  PORTB &= ~_BV(PIN3);

  irctrl_state = IRCTRL_STATE_NOP;
}

uint8_t ir_scan(char* params_str)
{
  uint8_t ret;

  if(params_str == NULL)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_REQUIRED, "param required");
  }

  uint16_t period; /* period in us */
  period = atoi(params_str);

  if(period <= 0)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_INVALID, "invalid param");
  }
  else if(period > 5000)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_TOO_LARGE,"param too large");
  }

  scan_state_t ss;
  scan_state = &ss;

  ret = start_scan(period);

  switch(ret)
  {
    case 0:
      break;
    case IRCTRL_ERR_PARAM_TOO_SMALL:
      RETURN_CMD_ERR_P(ret,"param too small");
    case IRCTRL_ERR_PARAM_TOO_LARGE:
      RETURN_CMD_ERR_P(ret,"param too large");
  }

  while(scan_state->end_flag==0); // wait for scan

  /* print result data */
  char buf[5];

  itoa(period,buf,10);
  print_P("period     : ");
  print(buf);
  println_P(" us");

  itoa(scan_state->data_count,buf,10);
  print_P("data length: ");
  println(buf);

  print_P("data       : ");
  for(uint16_t i=0; i<scan_state->data_count; i++)
  {
    uint16_t pos1, pos2;
    pos1 = i / 8;
    pos2 = i % 8;
    char c = (scan_state->data[pos1]&(1<<pos2)) ? '1' : '0';
    printc(c);
  }
  println_P("");

  scan_state = NULL;
  RETURN_CMD_OK;
}

uint8_t ir_send(char* params_str)
{
  uint8_t ret;

  // - check params
  if(params_str == NULL)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_REQUIRED, "param required");
  }

  uint16_t period; /* period in us */
  period = atoi(params_str);

  if(period <= 0)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_INVALID, "invalid param");
  }
  else if(period > 5000)
  {
    RETURN_CMD_ERR_P(IRCTRL_ERR_PARAM_TOO_LARGE,"param too large");
  }

  // - receive data
  char buf[512];
  print_P("data: ");
  getln(buf,512);
  println(buf);


  // - start carrier
  // - start timer
  // - wait for complete sending

  RETURN_CMD_OK;
}

uint8_t start_timer(uint16_t period)
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

void stop_timer()
{
  TCCR0B = 0x00; // stop timer
  TIMSK0 = 0x00; // stop interruption
}

uint8_t start_scan(uint16_t period)
{
  uint8_t ret;

  /* initialize vars */
  scan_state->end_flag = 0;
  scan_state->cont_count = 0;
  scan_state->data_count = 0;

  ret = start_timer(period);

  if(ret == 0) {
    irctrl_state = IRCTRL_STATE_SCAN;
  }

  return ret;
}

void stop_scan()
{
  stop_timer();
  irctrl_state = IRCTRL_STATE_NOP;
  scan_state->end_flag = 1;
}

// called every timer0A comp match */
ISR(TIMER0_COMPA_vect)
{
  switch(irctrl_state)
  {
    case IRCTRL_STATE_SCAN:
      isr_scan();
      break;
    case IRCTRL_STATE_SEND:
      isr_send();
      break;
  }
}

void isr_scan()
{
  uint8_t input;
  input = ((PIND & _BV(PIN7))>>PIN7) ^ 1;

  // wait for first input
  if(scan_state->data_count == 0  && input == 0)
  {
    return;
  }

  // detect end of signal
  if(input == 0)
  {
    scan_state->cont_count++;
    if(scan_state->cont_count == IRCTRL_END_CONT)
    {
      scan_state->data_count -= scan_state->cont_count - 1;
      stop_scan();
      return;
    }
  }
  else
  {
    scan_state->cont_count = 0;
  }

  uint8_t pos1, pos2;
  pos1 = scan_state->data_count / 8;
  pos2 = scan_state->data_count % 8;

  uint8_t data;
  data = scan_state->data[pos1];

  scan_state->data[pos1] = (data & ~(1<<pos2)) | (input<<pos2);

  scan_state->data_count++;

  if(scan_state->data_count == IRCTRL_BUF_LEN*8-1)
  {
    scan_state->data_count -= scan_state->cont_count;
    stop_scan();
  }

  return;

}

void isr_send()
{
  return;
}
