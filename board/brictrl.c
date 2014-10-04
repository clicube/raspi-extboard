#include <avr/io.h>
#include <stdlib.h>
#include "brictrl.h"
#include "command.h"


void bri_init()
{
  /* initialize register */
  /* REFS = 11: ADC reference is internal 1.1V
   * MUX = 0000: input ch is 0
   * */
  ADMUX = 0b11000000;
  return;
}

uint8_t bri_read(char* params_str)
{
  ADCSRA |= _BV(ADEN)|_BV(ADSC); /* enable & start ADC */
  while(ADCSRA & _BV(ADSC)); /* wait for ADC complete */

  volatile uint16_t result;
  result = ADCL;
  result += (ADCH<<8);

  char buf[6];

  ltoa(result,buf,10);
  print_P("BRI: ");
  println(buf);

  RETURN_CMD_OK;
}

