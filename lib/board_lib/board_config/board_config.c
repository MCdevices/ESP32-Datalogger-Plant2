#include "board_config/board_config.h"

char *TAG = "ADC READ";
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

void write_sd_card(sdmmc_card_t *card, FILE *file_open, char *file_data_write, char buf[], char mount_point[]){
    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "Opening file %s", file_data_write);
    file_open = fopen(file_data_write, "w");
    if (file_open == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(file_open, "AUTOMATED IRRIGATION!\nDate/time in Italy is: %s \n",buf); //card->cid.name,
    fclose(file_open);
    ESP_LOGI(TAG, "File written");

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}

void read_sd_card(char *file_data_read, FILE *file_open){
    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_data_read);
    file_open = fopen(file_data_read, "r");
    if (file_open == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fgets(line, sizeof(line), file_open);
    fclose(file_open);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}

//SLEEP MODE IN SEC
void deep_sleep_mode(uint8_t time_sleep_s){
    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", time_sleep_s);
    esp_wifi_stop(); //turn off wifi 
    esp_deep_sleep(1000000LL * time_sleep_s);
}