#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "task.h"
#define TAG "SDCARD"
extern char buffer[100];
// SPI Pins
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS    5

#define MOUNT_POINT "/sdcard"
void sdcard_mount_init(void){
    esp_err_t ret;
    ESP_LOGI(TAG, "Initializing SD card with SPI interface...");

    // SPI Bus Configuration
    spi_bus_config_t bus_config = {
        .mosi_io_num = SPI2_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz =4000,
    };

    spi_host_device_t spi_host = SPI2_HOST;

    ret = spi_bus_initialize(spi_host, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return;
    }

    // Optional delay to allow card to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));

    // Slot Configuration
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = spi_host;

    // Lower SPI frequency to improve compatibility
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = spi_host;
    host.max_freq_khz = 4000; 

    // Mount Configuration
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount filesystem. Error: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SD card mounted successfully.");
    sdmmc_card_print_info(stdout, card);
}
void app_main(void) {
    sdcard_mount_init();
    rtc_init();
    char* time =datetime();
    // File operations
    FILE *f = fopen(MOUNT_POINT "/task.csv", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing.");
        return;
    }
    fprintf(f, "%s,1\n",time);
    fclose(f);
    ESP_LOGI(TAG, "File written successfully.");
}






































// void app_main(void) {
//     // esp_err_t ret;
//     // ESP_LOGI(TAG, "Initializing SD card with SPI interface...");

//     // // SPI Bus Configuration
//     // spi_bus_config_t bus_config = {
//     //     .mosi_io_num = PIN_NUM_MOSI,
//     //     .miso_io_num = PIN_NUM_MISO,
//     //     .sclk_io_num = PIN_NUM_CLK,
//     //     .quadwp_io_num = -1,
//     //     .quadhd_io_num = -1,
//     //     .max_transfer_sz =4000,
//     // };

//     // spi_host_device_t spi_host = SPI2_HOST;

//     // ret = spi_bus_initialize(spi_host, &bus_config, SPI_DMA_CH_AUTO);
//     // if (ret != ESP_OK) {
//     //     ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
//     //     return;
//     // }

//     // // Optional delay to allow card to stabilize
//     // vTaskDelay(pdMS_TO_TICKS(100));

//     // // Slot Configuration
//     // sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
//     // slot_config.gpio_cs = PIN_NUM_CS;
//     // slot_config.host_id = spi_host;

//     // // Lower SPI frequency to improve compatibility
//     // sdmmc_host_t host = SDSPI_HOST_DEFAULT();
//     // host.slot = spi_host;
//     // host.max_freq_khz = 4000; 

//     // // Mount Configuration
//     // esp_vfs_fat_mount_config_t mount_config = {
//     //     .format_if_mount_failed = false,
//     //     .max_files = 5,
//     //     .allocation_unit_size = 16 * 1024
//     // };

//     // sdmmc_card_t *card;
//     // ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
//     // if (ret != ESP_OK) {
//     //     ESP_LOGE(TAG, "Failed to mount filesystem. Error: %s", esp_err_to_name(ret));
//     //     return;
//     // }

//     // ESP_LOGI(TAG, "SD card mounted successfully.");
//     // sdmmc_card_print_info(stdout, card);
//     mount_init();
//     // File operations
//     FILE *f = fopen(MOUNT_POINT "/test.txt", "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing.");
//         return;
//     }
//     fprintf(f, "Hello from ESP32 with SPI SD card!\n");
//     fclose(f);
//     ESP_LOGI(TAG, "File written successfully.");
// }






