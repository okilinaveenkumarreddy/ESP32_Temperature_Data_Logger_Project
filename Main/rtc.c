#include <stdio.h>
#include "task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include <time.h>
#include <string.h>
#define SCL 22
#define SDA 21
#define TAG "RTC"
#define DS3231 0x68
int int_to_bcd(int val);
int bcd_to_int(int val);
char buffer[100];
i2c_master_dev_handle_t rtc_handle;
i2c_master_bus_handle_t bus_handle;
void rtc_init()
{
    i2c_master_bus_config_t bus_config={
        .i2c_port=I2C_NUM_0,
        .clk_source=I2C_CLK_SRC_DEFAULT,
        .scl_io_num=SCL,
        .sda_io_num=SDA,
        .glitch_ignore_cnt=7,
        .flags.enable_internal_pullup=true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config,&bus_handle));


    i2c_device_config_t device_config={
        .dev_addr_length=I2C_ADDR_BIT_LEN_7,
        .device_address=DS3231,
        .scl_speed_hz=10000,
        .scl_wait_us=0,
    };

    esp_err_t err=i2c_master_probe(bus_handle,DS3231,-1);
    if (err==ESP_OK)
    {
        ESP_LOGI(TAG,"Found the RTC Module\n");
    }
    else
    {
        ESP_LOGW(TAG,"Unable to find RTC\n");
    }
    i2c_master_bus_add_device(bus_handle,&device_config,&rtc_handle);
}
char* datetime(){
    // uint8_t write_buffer[]={0x00,
    //     int_to_bcd(0),//seconds
    //     int_to_bcd(02),//min
    //     int_to_bcd(17),//hrs
    //     int_to_bcd(4),//wday
    //     int_to_bcd(10),//week date
    //     int_to_bcd(4),//mon
    //     int_to_bcd(25),
    //  };
    // ESP_ERROR_CHECK(i2c_master_transmit(device_handle,write_buffer,sizeof(write_buffer),-1));

    uint8_t write_buffer_for_read[]={0x00};
    uint8_t read_buffer[7];
    ESP_ERROR_CHECK(i2c_master_transmit_receive(rtc_handle,write_buffer_for_read,sizeof(write_buffer_for_read),read_buffer,sizeof(read_buffer),-1));
    struct tm dateTime={
        .tm_sec=bcd_to_int(read_buffer[0]),//sec
        .tm_min=bcd_to_int(read_buffer[1]),
        .tm_hour=bcd_to_int(read_buffer[2]),
        .tm_wday=bcd_to_int(read_buffer[3]),
        .tm_mday=bcd_to_int(read_buffer[4]),
        .tm_mon=bcd_to_int(read_buffer[5])-1,
        .tm_year=bcd_to_int(read_buffer[6])+100,
    };
    // char buffer[100];
    memset(buffer,0,sizeof(buffer));
    
    ESP_ERROR_CHECK(i2c_master_bus_rm_device(rtc_handle));
    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    strftime(buffer,sizeof(buffer),"%x %X",&dateTime);
    //printf("DATE TIME :%s\n",buffer);
    return buffer;
}
int int_to_bcd(int val)
{
    return ((val/10)<<4)|(val%10);
}
int bcd_to_int(int val)
{
    return (((val&0xF0)>>4)*10)+(val&0x0F);
}
