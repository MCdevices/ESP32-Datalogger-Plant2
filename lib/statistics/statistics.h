#ifndef STATISTICS_H
#define STATISTICS_H

#include "main.h"

typedef struct statistics
{
    uint32_t num_sample;
    uint8_t  hour;
    uint8_t  min;
    uint8_t  sec;
    uint16_t hum_value;

}Data_rec;

#endif