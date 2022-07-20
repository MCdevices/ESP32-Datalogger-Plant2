#ifndef DATAREC_CONFIG_H
#define DATAREC_CONFIG_H

#include "main.h"

void pin_config(void);
void check_efuse(void);
void read_sd_card(char *file_data_read, FILE *file_open);
void write_sd_card(sdmmc_card_t *card, FILE *file_open, char *file_data_write, char buf[],char mount_point[]);
void append_data_sd_card(sdmmc_card_t *card, FILE *file_open, char *file_data_write,uint16_t value ,char mount_point[]);
void spi_disable(sdmmc_card_t *card,char mount_point[]);
int32_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
void deep_sleep_mode(uint8_t time_sleep_s);

#endif