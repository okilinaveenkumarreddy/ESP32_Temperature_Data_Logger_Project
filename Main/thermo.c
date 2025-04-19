#include <stdio.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MAX31856"

// SPI pins
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   13

spi_device_handle_t ktype_handle;

void write_reg(uint8_t reg, uint8_t value) {
    spi_transaction_t t = {
        .length = 16,
        .flags = SPI_TRANS_USE_TXDATA,
        .tx_data = { (uint8_t)(reg | 0x80), value }
    };
    spi_device_transmit(ktype_handle, &t);
}

uint8_t read_reg(uint8_t reg) {
    spi_transaction_t t = {
        .length = 16,
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA,
        .tx_data = { reg & 0x7F, 0x00 }
    };
    spi_device_transmit(ktype_handle, &t);
    return t.rx_data[1];
}

float ktype_temp_read() {
    uint8_t t1 = read_reg(0x0C); // byte 1
    uint8_t t2 = read_reg(0x0D); // byte 2
    uint8_t t3 = read_reg(0x0E); // byte 3

    int32_t temp = ((t1 << 16) | (t2 << 8) | t3) >> 5; // 19-bit signed value

    // Sign extension if negative
    if (temp & 0x80000) {
        temp |= 0xFFF00000;
    }

    return temp * 0.0078125;  // LSB = 0.0078125 °C
}

void max31856_init() {
    // Set CR0: Conversion Mode: Auto, normal operation
    write_reg(0x00, 0x80);

    //Thermocouple Type = K (0x03)
    write_reg(0x01, 0x03);

    // Mask Faults Register 
    write_reg(0x02, 0x00);
}

void app_main() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI2_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 1,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &ktype_handle));

    max31856_init();
    ESP_LOGI(TAG, "MAX31856 Initialized");

    while (1) {
        float temp = ktype_temp_read();
        ESP_LOGI(TAG, "Thermocouple Temperature: %.2f °C", temp);
        // Optional: Read fault register
        uint8_t fault = read_reg(0x0F);
        if (fault != 0) {
            ESP_LOGW(TAG, "Fault detected! Fault register: 0x%02X", fault);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
    





// #include <stdio.h>
// #include <string.h>
// #include "esp_log.h"
// #include "driver/spi_master.h"
// #include "driver/gpio.h"
// #define MOSI 12
// #define MISO 13
// #define SCLK 15
// #define CS0 14
// uint8_t thermo_read(uint8_t reg);
// spi_device_handle_t device_handle;
// uint8_t msbit,midbit,lsbit;
// #define DRDY 13
// void app_main()
// {
//     spi_bus_config_t bus_config={
//         .miso_io_num=MISO,
//         .mosi_io_num=MOSI,
//         .sclk_io_num=SCLK,
//         .quadhd_io_num=-1,
//         .quadwp_io_num=-1,
//     };
//     spi_device_interface_config_t device_config={
//         .clock_speed_hz=1000000,
//         .spics_io_num=CS0,
//         .mode=3,
//         .queue_size=1,
//         .duty_cycle_pos=128,
//     };

//     ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST,&bus_config,SPI_DMA_CH_AUTO));
//     ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST,&device_config,&device_handle));
//     msbit=thermo_read(0x0C);
//     midbit=thermo_read(0x0D);
//     lsbit=thermo_read(0x0E);
//     uint32_t temp=(msbit<<16|midbit<<8|lsbit)>>5;
//     float ktype=temp*0.0078125;
//     printf("Temp Value:%.2f\n",ktype);
//     // spi_transaction_t action;
//     // memset(&action,0,sizeof(action));
//     // while(1)
//     // {
//     //     char send_buff[100];
//     //     snprintf(send_buff,sizeof(send_buff),"HII from ESP32");
//     //     char recv_buff[100];
//     //     action.length=100*8;//2 bytes
//     //     action.tx_buffer=send_buff;
//     //     action.rx_buffer=recv_buff;
//     //     ESP_ERROR_CHECK(spi_device_transmit(device_handle,&action));
//     //     printf("received %s\n",recv_buff);
//     //     vTaskDelay(pdMS_TO_TICKS(1000));

//     // }
    
// }
// uint8_t thermo_read(uint8_t reg)
// {
//     gpio_set_direction(DRDY,GPIO_MODE_INPUT);
//     while(gpio_get_level(DRDY)==1)
//     {
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
//     uint8_t send_buff[]={reg&0x7F,0x00};
//     uint8_t recv_buff[2]={0};
//     spi_transaction_t action;
//     memset(&action,0,sizeof(action));

//     action.length=2*8;//2 bytes
//     action.tx_buffer=send_buff;
//     action.rx_buffer=recv_buff;
//     action.flags = SPI_TRANS_USE_RXDATA;
//     ESP_ERROR_CHECK(spi_device_transmit(device_handle,&action));
//     vTaskDelay(pdMS_TO_TICKS(1000));
//     printf("temp read:%d\n",recv_buff[1]);
//     return recv_buff[1];
// }