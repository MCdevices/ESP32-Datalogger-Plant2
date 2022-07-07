#ifndef MAIN_H
#define MAIN_H

// ************************************************
// ******************* INCLUDES *******************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

//Driver
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

//Wifi - Time
#include <time.h>
#include <sys/time.h>
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"

//ESP32
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_event.h"

//MY INCLUDES
#include "board_config/board_config.h"


// ************************************************
// ***************** DEFINITIONS ******************
#define ADC2_CHAN0                  ADC2_CHANNEL_0              //ADC Channel GPIO4  
#define adc_atten                   ADC_ATTEN_DB_11             //ADC Attenuation       
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF  //ADC Calibration
#define LED_PIN                     2
#define RELAY_PIN                   5

//MAX-MIN VALUE CALIBRATION
#define MAX_VALUE 4095 //sensor in the air
#define MIN_VALUE 910  //sensor in the water "extreme condition"
#define NUM_MAX_SAMPLE 10

//WIFI
#define EXAMPLE_WIFI_SSID "Xperia XZ1_ad70"
#define EXAMPLE_WIFI_PASS "matteocava"


// ************************************************
// ****************** VARIABLE ********************
extern char *TAG;
extern char *TAG1;
esp_adc_cal_characteristics_t adc_chars;
EventGroupHandle_t wifi_event_group;
time_t now;
struct tm timeinfo;



#endif