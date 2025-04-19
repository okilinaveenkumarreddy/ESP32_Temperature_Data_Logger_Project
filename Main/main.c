#include<stdio.h>
#include "header.h"
#include <stdlib.h>
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define TAG "MAIN"


// SPI pins
#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_CLK  18
spi_device_handle_t ktype_handle;
spi_host_device_t spi_host = SPI2_HOST;
void spi_bus_init(){
    esp_err_t ret;
    // SPI bus_config
    spi_bus_config_t bus_config = {
            .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz =4000,
    };
    ret = spi_bus_initialize(spi_host, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}
void app_main(void)
{      /* Driver for Modules*/
    init_rtc();
    //set_time();
    spi_bus_init();
    sdcard_mount_init();
    ktype_device_init();
    ktype_write();
    ESP_LOGI(TAG, "Devices Intialized\n");
    while (1) {
        char* time =get_dateTime();
        float temp = ktype_temp_read();
        ESP_LOGI(TAG, "Time %s,Temperature: %.2f °C\n", time,temp);//for my understanding printing on idf terminal
        //File operations
        FILE *f = fopen(MOUNT_POINT "/task.csv", "a");
        if (f == NULL) {
            ESP_LOGE(TAG, "Failed to open file for writing.\n");
            return;
        }
        fprintf(f, "%s,%.2f\n",time,temp);
        fclose(f);
        ESP_LOGI(TAG, "File written successfully.\n\n");
        vTaskDelay(pdMS_TO_TICKS(20000)); //append the data for every twenty seconds
    }
}
