/*************************************************/
/*<                  MCDEVICES                  >*/
/*************************************************/
/*<             DATALOGGER FOR PLANTS           >*/
/*<                version: 1.0.1               >*/               
/*************************************************/

/*
NOTE:
    (1)utiliazzare un pin di uscira per pilotare un BJT,
    avra la funzione di accendere il sensore solo nel
    momento in cui si effettua la misurazione dell'
    umidita, questo fa in modo che il sensore non si 
    corrodi molto velocemente.

    (2)Nel momento in cui l'umiduta del terreno scende sotto 
    una soglia prestabilita verra effettuata l'irrigazione 
    del terreno tramite la pompa.
    (3)SEE: https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/,
    https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
*/

#include "main.h"
#include "statistics.h"
#include "wifi_connect.h"
// ************************************************
// **************** TIMER HANDLE ******************
static TimerHandle_t scheduler_handle_id;
static TimerHandle_t minutes_handle_id;

// ************************************************
// ***************** VARIABLES ********************
static uint16_t adc_raw;
static uint16_t voltage;
static uint16_t percentuale;
static uint32_t num_samp = 0;
static uint32_t minutesCounter = 0;
unsigned long actual_timestamp = 0;
RTC_DATA_ATTR static int boot_count = 0;

Data_rec oneButterTx[10];
esp_err_t ret = ESP_OK;
esp_err_t event_handler(void *ctx, system_event_t *event);

void repeated_timer_handler_minute(TimerHandle_t xTimer){   //FOR DEBUGGING ONLY!!!
    ++boot_count;
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    char strftime_buf[64];

    // Set timezone to Eastern Standard Time and print local time ===> EXAMPLE
    setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);

    // Set timezone to Italy Standard Time
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Italy is: %s", strftime_buf);

    // const int deep_sleep_sec = 10;
    // ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
    // esp_deep_sleep(1000000LL * deep_sleep_sec);
}

static void scheduler_thread(TimerHandle_t xTimer){
    bool cali_enable = adc_calibration_init();
    //ADC2 config
    gpio_set_level(PIN_LED,1);
    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_CHAN0, adc_atten));
    ret = adc2_get_raw(ADC2_CHAN0, ADC_WIDTH_BIT_DEFAULT, &adc_raw); //read on single channel
    ESP_ERROR_CHECK(ret);
    if (cali_enable) {
        voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc_chars);
        ESP_LOGI(TAG,"raw  data: %d", adc_raw); //FOR DEBUGGING ONLY!!!
        if(adc_raw == 0){
            voltage = 0;
        }
        ESP_LOGI(TAG, "voltage: %d mV", voltage); //FOR DEBUGGING ONLY!!!
        percentuale = map(adc_raw,MIN_VALUE,MAX_VALUE,100,0);
        ESP_LOGI(TAG, "percentuale: %d", percentuale);   //prints the percentage of soil moisture
    }
    num_samp++;
    oneButterTx[10].num_sample = num_samp;
    ESP_LOGI(TAG, "Number of samples: %d ", oneButterTx[10].num_sample);
    gpio_set_level(PIN_LED,0);
}

static void create_timers(void){
    /*FREERTOS TIMER CREATE*/
    scheduler_handle_id = xTimerCreate("SCHED",                  //Timer Name
                                        pdMS_TO_TICKS(1000),     //Period in ticks
                                        pdTRUE,                 //Auto reload
                                        NULL,                   //timer ID
                                        &scheduler_thread);      //Callback Function
    
    // minutes_handle_id   = xTimerCreate("SCHED",                  //Timer Name
    //                                     pdMS_TO_TICKS(60000),     //Period in ticks
    //                                     pdTRUE,                 //Auto reload
    //                                     NULL,                   //timer ID
    //                                     &repeated_timer_handler_minute);      //Callback Function

    /* Error checking */
    if (NULL == scheduler_handle_id) // ||(NULL == minutes_handle_id)
    {
        ESP_LOGW(TAG, "ESP32_ERROR_MEM");
    }

    xTimerStart(scheduler_handle_id,0);
    //xTimerStart(minutes_handle_id,0);
}


void app_main(void){
    repeated_timer_handler_minute(NULL);
    //initialize.
    pin_config();
    create_timers();

    while (1) {
        vTaskDelay(5);
    }
}

