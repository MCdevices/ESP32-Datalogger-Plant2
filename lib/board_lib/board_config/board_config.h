#ifndef DATAREC_CONFIG_H
#define DATAREC_CONFIG_H

#include "main.h"

void pin_config(void);
void check_efuse(void);
void read_sd_card(char *file_data_read, FILE *file_open);
void write_sd_card(FILE *file_to_open, char *file_name ,char mount_point[], char bufTime[], int32_t perc, uint32_t num_samp);
void spi_disable(sdmmc_card_t *card,char mount_point[]);
int32_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
void deep_sleep_mode(uint8_t time_sleep_s);

#endif