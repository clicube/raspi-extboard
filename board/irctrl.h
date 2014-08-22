#ifndef IRCTRL_H
#define IRCTRL_H

/*
 * this module uses Timer0.
 */

#define IRCTRL_ERR_PARAM_INVALID   255
#define IRCTRL_ERR_PARAM_TOO_SMALL 254
#define IRCTRL_ERR_PARAM_TOO_LARGE 253

void ir_init(void);
uint8_t ir_scan(char*);

#endif
