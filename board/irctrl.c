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

#define IRCTRL_SEND_OPERATION_L 0
#define IRCTRL_SEND_OPERATION_H 1
#define IRCTRL_SEND_OPERATION_STOP 2

#define IRCTRL_SEND_STATE_START 0
#define IRCTRL_SEND_STATE_SEND_L 1
#define IRCTRL_SEND_STATE_SEND_H 2
#define IRCTRL_SEND_STATE_SEND_0_BIT0 3
#define IRCTRL_SEND_STATE_SEND_0_BIT1 4
#define IRCTRL_SEND_STATE_SEND_1_BIT0 5
#define IRCTRL_SEND_STATE_SEND_1_BIT1 6
#define IRCTRL_SEND_STATE_SEND_1_BIT2 7
#define IRCTRL_SEND_STATE_SEND_1_BIT3 8

//#define IRCTRL_VERIFY_OUTPUT
#define IRCTRL_VERIFY_OK 255
#define IRCTRL_VERIFY_NG 254


uint8_t start_timer(uint16_t period);
static inline void stop_timer();
static inline uint8_t start_scan(uint16_t period);
static inline void stop_scan(void);
static inline uint8_t start_send(uint16_t period);
static inline void stop_send(void);
static inline void isr_scan(void);
static inline void isr_send(void);

struct _scan_state_t
{
  volatile uint8_t end_flag;
  uint16_t cont_count;
  uint16_t data_count;
  uint8_t data[IRCTRL_SCAN_BUF_LEN];
};
typedef struct _scan_state_t scan_state_t;

struct _send_state_t
{
  volatile uint8_t end_flag;
  uint8_t ocr2a_L;
  uint8_t ocr2a_H;
  uint8_t state;
  uint16_t cont_remain;
  uint8_t next_operation;
  char data[IRCTRL_SEND_BUF_LEN];
  char* data_ptr;
  uint8_t sending_char;
  uint8_t sending_bit;
  uint8_t OCR2B_H;
#ifdef IRCTRL_VERIFY_OUTPUT
  uint8_t current_operation;
  uint8_t verify_state;
  uint16_t sent_count;
#endif
};
typedef struct _send_state_t send_state_t;

uint8_t irctrl_state;
scan_state_t* scan_state;
send_state_t* send_state;


void ir_init()
{
  /* Pin initialization */
  /* set PD7 to input, Hi-Z */
  DDRD  &= ~_BV(PIN7);
  PORTD &= ~_BV(PIN7);

  /* set PB3 to output, Low */
  DDRD  |= _BV(PIN3);
  PORTD &= ~_BV(PIN3);

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
  send_state_t ss;
  send_state = &ss;

  print_P("data: ");
  getln(send_state->data, IRCTRL_SEND_BUF_LEN);

  // send
  ret = start_send(period);

  switch(ret)
  {
    case 0:
      break;
    case IRCTRL_ERR_PARAM_TOO_SMALL:
      RETURN_CMD_ERR_P(ret,"param too small");
    case IRCTRL_ERR_PARAM_TOO_LARGE:
      RETURN_CMD_ERR_P(ret,"param too large");
  }

  // - wait for complete sending
  while(send_state->end_flag==0); // wait for scan

#ifdef IRCTRL_VERIFY_OUTPUT
  switch(send_state->verify_state)
  {
    case IRCTRL_VERIFY_OK:
      println_P("verify: OK");
      break;
    case IRCTRL_VERIFY_NG:
      println_P("verify: NG");
      print_P("sent count: ");
      char str[6];
      itoa(send_state->sent_count, str, 10);
      println(str);
      break;
    default:
      println_P("verify state is invalid.");
  }

#endif

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

uint8_t start_send(uint16_t period)
{
  uint8_t ret;

  /* initialize vars */
  send_state->end_flag = 0;
  send_state->state = IRCTRL_SEND_STATE_START;
  send_state->next_operation = IRCTRL_SEND_OPERATION_L;
  send_state->cont_remain = 1;
  send_state->ocr2a_L = 0;
  send_state->ocr2a_H = 0; /* set lator */
  send_state->data_ptr = send_state->data;
  send_state->sending_char = 0xFF;
  send_state->sending_bit = 0;

#ifdef IRCTRL_VERIFY_OUTPUT
  send_state->current_operation = IRCTRL_SEND_OPERATION_L;
  send_state->verify_state = 0;
  send_state->sent_count = 0;
#endif


  /* calculation OCR2A ( TOP value for timer2 )
   *         and CS2n  ( N: division raito for timer2 )
   *
   * period = 1/freq
   * freq = F_CPU/(N*(TOP+1))
   * TOP = period*F_CPU/N-1
   *
   * freq = 38kHz
   * TOP = (1/38k)*F_CPU/N-1
   */

  uint16_t tmp;
  uint8_t top;
  uint8_t cs2n;

  tmp = F_CPU / IRCTRL_FREQ;

  if( tmp <= 256 )
  {
    // N = 1
    cs2n = 0b001;
    top = tmp/1-1;
  }
  else
  {
    // N = 8
    cs2n = 0b010;
    top = tmp/8-1;
    if(F_CPU/(8*(top+1))-IRCTRL_FREQ > IRCTRL_FREQ-F_CPU/(8*(top+2)))
    {
      top++;
    }

  }

  /* set Timer 2
   * Timer mode: Highspeed PWM (WGM2n = 0b111)
   * Output B mode: non-inverted mode (COM2Bn = 0b10) => output on
   *                or normal mode    (COM2Bn = 0b00) => output off
   * No interruption
   *
   * ==> start timer
   */
  //TCCR2A = 0b00000011;
  TCCR2A = 0b00110011;
  TCCR2B = 0b00001000 | cs2n;
  OCR2A = top;
  send_state->OCR2B_H = top/3*2;
  //OCR2B = top/3;
  OCR2B = 255;
  TIMSK2 = 0x00;

  /* start timer */
  ret = start_timer(period);

  if(ret == 0) {
    irctrl_state = IRCTRL_STATE_SEND;
  }

  return ret;
}

void stop_send()
{
  stop_timer();

  /* stop carrier */
  TCCR2A &= ~0b00110000;

  send_state->end_flag = 1;
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

  if(scan_state->data_count == IRCTRL_SCAN_BUF_LEN*8-1)
  {
    scan_state->data_count -= scan_state->cont_count;
    stop_scan();
  }

  return;

}

void isr_send()
{
#ifdef IRCTRL_VERIFY_OUTPUT
  uint8_t input;

  input = ((PIND & _BV(PIN7))>>PIN7) ^ 1;
  if(
      ( send_state->current_operation == IRCTRL_SEND_OPERATION_L && input != 0 ) ||
      ( send_state->current_operation == IRCTRL_SEND_OPERATION_H && input != 1 )
    )
  {
    send_state->verify_state = IRCTRL_VERIFY_NG;
    stop_send();
    return;
  }
  if( send_state->next_operation == IRCTRL_SEND_OPERATION_STOP )
  {
    send_state->verify_state = IRCTRL_VERIFY_OK;
  }
#endif

  /* change output */
  switch(send_state->next_operation)
  {
    case IRCTRL_SEND_OPERATION_L:
      //TCCR2A &= ~0b00110000;
      OCR2B = 255;
      break;
    case IRCTRL_SEND_OPERATION_H:
      //TCCR2A |= 0b00100000;
      OCR2B = send_state->OCR2B_H;
      break;
    case IRCTRL_SEND_OPERATION_STOP:
      stop_send();
      return;
  }

#ifdef IRCTRL_VERIFY_OUTPUT
  send_state->verify_state = 1;
  send_state->current_operation = send_state->next_operation;
  send_state->sent_count += 1;
#endif


  /* set next operation */
  char c;
  switch(send_state->state)
  {
    case IRCTRL_SEND_STATE_SEND_L:
    case IRCTRL_SEND_STATE_SEND_H:
      send_state->cont_remain--;
      if(send_state->cont_remain > 0)
      {
        // keep next_operation
        break;
      }

    case IRCTRL_SEND_STATE_START:
    case IRCTRL_SEND_STATE_SEND_0_BIT1:
    case IRCTRL_SEND_STATE_SEND_1_BIT3:
      if(send_state->sending_char == 0xFF || send_state->sending_bit == 3)
      {
        c = *(send_state->data_ptr);
        switch(c)
        {
          case '\0':
            send_state->next_operation = IRCTRL_SEND_OPERATION_STOP;
            break;
          case 'L':
          case 'H':
            if(*(send_state->data_ptr+1) == '(')
            {
              /* replace ')' to '\0' */
              char* tmp = send_state->data_ptr+1;
              while( *tmp != ')' && *tmp != '\0'){ tmp++; }
              if(*tmp == '\0') /* unexpected end of string */
              {
                stop_send();
                return;
              }
              *tmp = '\0';

              uint16_t cont;
              cont = atoi(send_state->data_ptr+2);
              if(cont == 0)
              {
                stop_send();
                return;
              }
              send_state->cont_remain = cont;
              send_state->data_ptr = tmp;
            }
            else
            {
              send_state->cont_remain = 1;
            }
            switch(c)
            {
              case 'L':
                send_state->state = IRCTRL_SEND_STATE_SEND_L;
                send_state->next_operation = IRCTRL_SEND_OPERATION_L;
                break;
              case 'H':
                send_state->state = IRCTRL_SEND_STATE_SEND_H;
                send_state->next_operation = IRCTRL_SEND_OPERATION_H;
                break;
            }
            break;
          default:
            switch(c){
              case '0':
                send_state->sending_char = 0x0;
                break;
              case '1':
                send_state->sending_char = 0x1;
                break;
              case '2':
                send_state->sending_char = 0x2;
                break;
              case '3':
                send_state->sending_char = 0x3;
                break;
              case '4':
                send_state->sending_char = 0x4;
                break;
              case '5':
                send_state->sending_char = 0x5;
                break;
              case '6':
                send_state->sending_char = 0x6;
                break;
              case '7':
                send_state->sending_char = 0x7;
                break;
              case '8':
                send_state->sending_char = 0x8;
                break;
              case '9':
                send_state->sending_char = 0x9;
                break;
              case 'A':
                send_state->sending_char = 0xA;
                break;
              case 'B':
                send_state->sending_char = 0xB;
                break;
              case 'C':
                send_state->sending_char = 0xC;
                break;
              case 'D':
                send_state->sending_char = 0xD;
                break;
              case 'E':
                send_state->sending_char = 0xE;
                break;
              case 'F':
                send_state->sending_char = 0xF;
                break;
              default:
                send_state->sending_char = 0xFF;
                /* nop */
            }
            if(send_state->sending_char <= 0xF)
            {
              send_state->sending_bit = 0;
              if(send_state->sending_char & 1<<3)
              {
                send_state->state = IRCTRL_SEND_STATE_SEND_1_BIT0;
                send_state->next_operation = IRCTRL_SEND_OPERATION_H;
              }
              else
              {
                send_state->state = IRCTRL_SEND_STATE_SEND_0_BIT0;
                send_state->next_operation = IRCTRL_SEND_OPERATION_H;
              }
            }
        }
        send_state->data_ptr++;
      }
      else /* sending_char != 0xFF && sending_bit < 3 */
      {
        send_state->sending_bit++;
        if(send_state->sending_char & 1<<(3-send_state->sending_bit))
        {
          send_state->state = IRCTRL_SEND_STATE_SEND_1_BIT0;
          send_state->next_operation = IRCTRL_SEND_OPERATION_H;
        }
        else
        {
          send_state->state = IRCTRL_SEND_STATE_SEND_0_BIT0;
          send_state->next_operation = IRCTRL_SEND_OPERATION_H;
        }
        if(send_state->sending_bit == 3)
        {
          send_state->sending_char = 0xFF;
          send_state->sending_bit = 0;
        }
      }
      break;

    case IRCTRL_SEND_STATE_SEND_0_BIT0:
      send_state->state = IRCTRL_SEND_STATE_SEND_0_BIT1;
      send_state->next_operation = IRCTRL_SEND_OPERATION_L;
      break;

    case IRCTRL_SEND_STATE_SEND_1_BIT0:
      send_state->state = IRCTRL_SEND_STATE_SEND_1_BIT1;
      send_state->next_operation = IRCTRL_SEND_OPERATION_L;
      break;

    case IRCTRL_SEND_STATE_SEND_1_BIT1:
      send_state->state = IRCTRL_SEND_STATE_SEND_1_BIT2;
      send_state->next_operation = IRCTRL_SEND_OPERATION_L;
      break;

    case IRCTRL_SEND_STATE_SEND_1_BIT2:
      send_state->state = IRCTRL_SEND_STATE_SEND_1_BIT3;
      send_state->next_operation = IRCTRL_SEND_OPERATION_L;
      break;
  }

  return;
}
