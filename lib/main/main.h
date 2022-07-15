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

//SD CARD
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

//MY INCLUDES
#include "board_config/board_config.h"

// ************************************************
// ***************** DEFINITIONS ******************
#define LED_PIN                     21 //GPIO 21 on LilyGO/T8
#define RELAY_PIN                   2   //GPIO 5
#define SLEEP_TIME                  144*600 //144*600 = 86400 = 24H //il micro si risveglia ogni 24H
#define DEFAULT_VREF                1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES               64          //Multisampling

//MAX-MIN VALUE CALIBRATION
#define MAX_VALUE                   2681  //sensor in the air
#define MIN_VALUE                   984  //sensor in the water "extreme condition"
#define NUM_MAX_SAMPLE              20
#define TIME_FOR_IRRIG              5000 //in ms

//WIFI
#define EXAMPLE_WIFI_SSID           "Xperia XZ1_ad70"
#define EXAMPLE_WIFI_PASS           "matteocava"

//SDCARD SPI
#define MOUNT_POINT                 "/sdcard"
#define SPI_DMA_CHAN                1
#define PIN_NUM_MISO                2
#define PIN_NUM_MOSI                15
#define PIN_NUM_CLK                 14
#define PIN_NUM_CS                  13

// ************************************************
// ****************** VARIABLE ********************
extern char *TAG;
extern char *TAG1;
EventGroupHandle_t wifi_event_group;
time_t now;
struct tm timeinfo;


#endif