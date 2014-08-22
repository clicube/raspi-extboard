#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include "lib/uart/uart.h"
#include "lib/i2cmaster/i2cmaster.h"
#include "tempctrl.h"
#include "command.h"

#define ADDR_AM2321 0b10111000

uint8_t read_data(uint16_t*, uint16_t*);

void temp_init()
{
  _delay_ms(500);
}

uint8_t temp_read(char* params_str)
{

  uint8_t result;
  uint16_t temp, hum;
  result = read_data(&temp,&hum);
  if(result > 0)
  {
    CMD_ERR_P(255, "error in 1st read");
  }
  _delay_ms(2000);

  result = read_data(&temp,&hum);
  if(result > 0)
  {
    CMD_ERR_P(255, "error in 2nd read");
  }

  char buf[6];

  itoa(temp,buf,10);
  uart_puts_P("TMP: ");
  uart_puts(buf);
  uart_puts_P("\r\n");

  itoa(hum,buf,10);
  uart_puts_P("HUM: ");
  uart_puts(buf);
  uart_puts_P("\r\n");

  CMD_OK;
}

uint8_t read_data(uint16_t* temp, uint16_t* hum)
{

  // wakes up sensor
  i2c_start(ADDR_AM2321+I2C_WRITE);
  _delay_us(800);
  i2c_stop();

  uint8_t val;

  // send command
  val = i2c_start(ADDR_AM2321+I2C_WRITE);
  if(val > 0)
  {
    i2c_stop();
    return 255;
  }
  i2c_write(0x03);
  i2c_write(0x00);
  i2c_write(0x04);
  i2c_stop();

  _delay_us(1500);

  // receive data
  i2c_start_wait(ADDR_AM2321+I2C_READ);
  _delay_us(30);

  val = i2c_readAck();
  if(val != 0x03)
  {
    i2c_stop();
    return 254;
  }

  val = i2c_readAck();
  if(val != 0x04)
  {
    i2c_stop();
    return 253;
  }

  *hum = (i2c_readAck() << 8);
  *hum += i2c_readAck();

  *temp = (i2c_readAck() << 8);
  *temp += i2c_readAck();

  // read CRC check sum, but not used.
  i2c_readAck();
  i2c_readNak();
  i2c_stop();

  return 0;
}

