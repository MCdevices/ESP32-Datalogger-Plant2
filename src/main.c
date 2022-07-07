/*************************************************/
/*<                  MCDEVICES                  >*/
/*************************************************/
/*<             AUTOMATED IRRIGATION            >*/
/*<                version: 1.1.1               >*/               
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
    for timezione
*/

#include "main.h"
#include "wifi_connect.h"
// ************************************************
// **************** TIMER HANDLE ******************
static TimerHandle_t read_from_adc_handle_id;

// ************************************************
// ***************** VARIABLES ********************
static uint16_t adc_raw;
static uint16_t voltage;
static uint16_t percentuale;
unsigned long actual_timestamp = 0;
RTC_DATA_ATTR static int num_samp = 0;


time_t now;
struct tm timeinfo;
char *TAG1 = "WIFI";
esp_err_t ret = ESP_OK;
esp_err_t event_handler(void *ctx, system_event_t *event);


void get_time_date(TimerHandle_t xTimer){   //FOR DEBUGGING ONLY!!!
    char strftime_buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    //if the date is not initialized
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG1, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    // Set timezone to Italy Standard Time
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); //see esp32-idf guide
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG1, "Date/time in Italy is: %s", strftime_buf);
}

static void read_from_adc(TimerHandle_t xTimer){
    num_samp++;
    bool cali_enable = adc_calibration_init();
    //ADC2 config
    gpio_set_level(LED_PIN,1);

    ESP_ERROR_CHECK(adc2_config_channel_atten(ADC2_CHAN0, adc_atten));
    ret = adc2_get_raw(ADC2_CHAN0, ADC_WIDTH_BIT_DEFAULT, &adc_raw); //read on single channel
    ESP_ERROR_CHECK(ret);
    if (cali_enable) {
        voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc_chars);
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_LOGI(TAG,"raw  data: %d", adc_raw); //FOR DEBUGGING ONLY!!!
        if(adc_raw == 0){
            voltage = 0;
        }
        ESP_LOGI(TAG, "voltage: %d mV", voltage); //FOR DEBUGGING ONLY!!!
        percentuale = map(adc_raw,MIN_VALUE,MAX_VALUE,100,0);
        ESP_LOGI(TAG, "percentuale: %d", percentuale);   //prints the percentage of soil moisture
    }
    ESP_LOGI(TAG, "Number of samples: %d ", num_samp);
    gpio_set_level(LED_PIN,0);
    deep_sleep_mode(5); //boot without deleting the time
}

static void create_timers(void){
    /*FREERTOS TIMER CREATE*/
    read_from_adc_handle_id = xTimerCreate("SCHED",                //Timer Name
                                           1,                      //Period in ticks
                                           false,                  //Auto reload
                                           NULL,                   //timer ID
                                           &read_from_adc);        //Callback Function
    
    /* Error checking */
    if (NULL == read_from_adc_handle_id) 
    {
        ESP_LOGW(TAG, "ESP32_ERROR_MEM");
    }

    xTimerStart(read_from_adc_handle_id,0);
}


void app_main(void){
    //initialize.
    pin_config();
    get_time_date(NULL);
    create_timers();

    while (1) {
        vTaskDelay(5); //for watchdog timer only (WDT) 
    }
}

