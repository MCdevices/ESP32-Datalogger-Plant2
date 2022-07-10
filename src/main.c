/*************************************************/
/*<                  MCDEVICES                  >*/
/*************************************************/
/*<             AUTOMATED IRRIGATION            >*/
/*<                version: 1.2.0               >*/               
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

    (4)inserire le librerie per scrivere il tempo e il giorno in cui viene 
    effettuata l'irrigazione
    (solo se l'umidita del terreno e scesa sotto ad una determinata soglia)
*/

#include "main.h"
#include "wifi_connect.h"

// ************************************************
// **************** TIMER HANDLE ******************
static TimerHandle_t read_from_adc_handle_id;

// ************************************************
// ***************** VARIABLES ********************
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
RTC_DATA_ATTR static int num_samp = 0;

// time_t now;
// struct tm timeinfo;
char *TAG1 = "WIFI";
sdmmc_card_t *card;
esp_err_t ret = ESP_OK;
esp_err_t event_handler(void *ctx, system_event_t *event);
// First create a file.
const char *file_data = MOUNT_POINT"/data.txt";
// Read a line from file
char line[64];
char strftime_buf[64];

uint32_t voltage;
uint32_t adc_reading = 0;

void get_time_date(TimerHandle_t xTimer){   //FOR DEBUGGING ONLY!!!
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


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static void adc_reading_thread(TimerHandle_t xTimer){
    //Check if Two Point or Vref are burned into eFuse

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }
    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    //print_char_val_type(val_type); <=== FOR DEBUG

    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));

}

static void create_timers(void){
    /*FREERTOS TIMER CREATE*/
    read_from_adc_handle_id = xTimerCreate("SCHED",                //Timer Name
                                           pdMS_TO_TICKS(1000),    //Period in ticks
                                           pdTRUE,                 //Auto reload
                                           NULL,                   //timer ID
                                           &adc_reading_thread);   //Callback Function
    
    /* Error checking */
    if (NULL == read_from_adc_handle_id) 
    {
        ESP_LOGW(TAG, "ESP32_ERROR_MEM");
    }

    xTimerStart(read_from_adc_handle_id,0);
}

static void sd_card(void){
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "Opening file %s", file_data);
    FILE *f = fopen(file_data, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "MATTEO %s!, Date/time in Italy is: %s \n", card->cid.name,strftime_buf);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Open file for reading
    ESP_LOGI(TAG, "Reading file %s", file_data);
    f = fopen(file_data, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}

void app_main(void){
    //initialize.
    pin_config();
    get_time_date(NULL);
    check_efuse();
    sd_card();
    //create_timers();

    while (1) {
        vTaskDelay(5); //for watchdog timer only (WDT) 
    }
}
