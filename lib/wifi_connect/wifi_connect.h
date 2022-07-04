#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include "main.h"

void obtain_time(void);
void initialize_sntp(void);
void initialise_wifi(void);
esp_err_t event_handler(void *ctx, system_event_t *event);

#endif