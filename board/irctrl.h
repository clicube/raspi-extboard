#ifndef IRCTRL_H
#define IRCTRL_H

/*
 * this module uses Timer0.
 */

#define IRCTRL_SCAN_BUF_LEN 256
#define IRCTRL_END_CONT 0

#define IRCTRL_SEND_BUF_LEN 384

#define IRCTRL_ERR_PARAM_REQUIRED  255
#define IRCTRL_ERR_PARAM_INVALID   254
#define IRCTRL_ERR_PARAM_TOO_SMALL 253
#define IRCTRL_ERR_PARAM_TOO_LARGE 252

void ir_init(void);
uint8_t ir_scan(char*);
uint8_t ir_send(char*);

#endif
