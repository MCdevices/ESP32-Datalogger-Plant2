#ifndef DATAREC_CONFIG_H
#define DATAREC_CONFIG_H

#include "main.h"

bool adc_calibration_init(void);
void pin_config(void);
int32_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
void deep_sleep_mode(uint8_t time_sleep_s);

#endif