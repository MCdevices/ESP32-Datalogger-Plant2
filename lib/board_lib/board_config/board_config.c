#include "board_config/board_config.h"

char *TAG = "ADC READ";
char *TAG2 = "SD CARD";
char line[64];

void pin_config(void){
    gpio_reset_pin(LED_PIN);    //GPIO2
    gpio_reset_pin(RELAY_PIN);  //GPIO5
    gpio_set_direction(LED_PIN,GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_PIN,GPIO_MODE_OUTPUT);
}

void check_efuse(void){
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }

}

int32_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void spi_disable(sdmmc_card_t *card,char mount_point[]){
    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG2, "Card unmounted");
}

void write_sd_card(FILE *file_to_open, char *file_name ,char mount_point[], char bufTime[], int32_t perc, uint32_t num_samp){
    xTimerStop(read_from_adc_handle_id,0);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    ESP_LOGI(TAG2, "Opening file %s", file_name);
    file_to_open = fopen(file_name, "a");
    if (file_to_open == NULL) {
        ESP_LOGE(TAG2, "Failed to open file for writing");
        return;
    }
    fprintf(file_to_open, "Date and time: %d-%02d-%02d %02d:%02d:%02d	percentuale acqua: %d%%	Numero campioni: %d\n",tm.tm_mday, tm.tm_mon + 1,tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, perc, num_samp);
    fclose(file_to_open);
    ESP_LOGI(TAG2, "File create and written");

    
    xTimerStart(read_from_adc_handle_id,0);
}

void read_sd_card(char *file_data_read, FILE *file_open){
    // Open file for reading
    ESP_LOGI(TAG2, "Reading file %s", file_data_read);
    file_open = fopen(file_data_read, "r");
    if (file_open == NULL) {
        ESP_LOGE(TAG2, "Failed to open file for reading");
        return;
    }

    fgets(line, sizeof(line), file_open);
    fclose(file_open);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG2, "Read from file: '%s'", line);
}

//SLEEP MODE IN SEC
void deep_sleep_mode(uint8_t time_sleep_s){
    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", time_sleep_s);
    esp_wifi_stop(); //turn off wifi 
    esp_deep_sleep(1000000LL * time_sleep_s);
}