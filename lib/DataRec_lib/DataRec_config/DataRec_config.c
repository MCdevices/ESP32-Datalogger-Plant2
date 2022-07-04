#include "DataRec_config/DataRec_config.h"

char *TAG = "ADC READ";

void pin_config(void){
    gpio_reset_pin(PIN_LED);
    gpio_set_direction(PIN_LED,GPIO_MODE_OUTPUT);
}

void led_toggle(void){
    gpio_set_level(PIN_LED,1);
    vTaskDelay(100);
    gpio_set_level(PIN_LED,0);
}

bool adc_calibration_init(void){
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_2, adc_atten, ADC_WIDTH_BIT_DEFAULT, 0, &adc_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }
    return cali_enable;
}

int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//SLEEP MODE
void deep_sleep_mode(uint8_t time_sleep_s){
    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", time_sleep_s);
    esp_wifi_stop(); //turn off wifi 
    esp_deep_sleep(1000000LL * time_sleep_s);
}