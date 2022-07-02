/*************************************************/
/*<                  MCDEVICES                  >*/
/*************************************************/
/*<             DATALOGGER FOR PLANTS           >*/
/*<                version: 1.0.1               >*/               
/*************************************************/

#include "main.h"
#include "statistics.h"

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
Data_rec oneButterTx[10];
esp_err_t ret = ESP_OK;

void repeated_timer_handler_minute(TimerHandle_t xTimer){   //FOR DEBUGGING ONLY!!!
    ++minutesCounter;
    ESP_LOGI(TAG,"MAIN min counted \n\r");
    actual_timestamp += 60;
    ESP_LOGI(TAG,"MAIN Current time is %ld\n\r", actual_timestamp);
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
        ESP_LOGI(TAG, "%d", percentuale);   //prints the percentage of soil moisture
    }
    num_samp++;
    oneButterTx[10].num_sample = num_samp;
    ESP_LOGI(TAG, "Number of samples: %d ", oneButterTx[10].num_sample);
    gpio_set_level(PIN_LED,0);
}

static void create_timers(void){
    /*FREERTOS TIMER CREATE*/
    scheduler_handle_id = xTimerCreate("SCHED",                  //Timer Name
                                        pdMS_TO_TICKS(200),     //Period in ticks
                                        pdTRUE,                 //Auto reload
                                        NULL,                   //timer ID
                                        &scheduler_thread);      //Callback Function
    
    minutes_handle_id   = xTimerCreate("SCHED",                  //Timer Name
                                        pdMS_TO_TICKS(60000),     //Period in ticks
                                        pdTRUE,                 //Auto reload
                                        NULL,                   //timer ID
                                        &repeated_timer_handler_minute);      //Callback Function

    /* Error checking */
    if ((NULL == scheduler_handle_id)
        ||(NULL == minutes_handle_id))
    {
        ESP_LOGW(TAG, "ESP32_ERROR_MEM");
    }

    xTimerStart(scheduler_handle_id,0);
    xTimerStart(minutes_handle_id,0);
}

void app_main(void){
    //initialize.
    pin_config();
    create_timers();
    //vTaskStartScheduler();
    while (1) {
        vTaskDelay(10);
    }
}