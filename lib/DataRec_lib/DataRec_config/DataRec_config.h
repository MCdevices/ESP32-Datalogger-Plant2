#ifndef DATAREC_CONFIG_H
#define DATAREC_CONFIG_H

#include "main.h"

bool adc_calibration_init(void);
void pin_config(void);
void led_toggle(void);
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
void deep_sleep_mode(uint8_t time_sleep_s);

#endif