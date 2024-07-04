#include <stdio.h>
#include "bme680.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#define IIC_Dev  "/dev/i2c-1"
#define channel 1
#define USESPISINGLEREADWRITE 0 

int fd;

void user_delay_ms(uint32_t period)
{
    usleep(period*1000);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
    write(fd, &reg_addr,1);
    read(fd, reg_data, len);
    return rslt;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */
    int8_t *buf;
    buf = malloc(len +1);
    buf[0] = reg_addr;
    memcpy(buf +1, reg_data, len);
    write(fd, buf, len +1);
    free(buf);
    return rslt;
}

int main(int argc, char const *argv[])
{
    struct bme680_dev gas_sensor;
    if ((fd = open(IIC_Dev, O_RDWR)) < 0) {
        printf("Failed to open the i2c bus %s", argv[1]);
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, 0x77) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    gas_sensor.dev_id = BME680_I2C_ADDR_SECONDARY;
    gas_sensor.intf = BME680_I2C_INTF;
    gas_sensor.read = user_i2c_read;
    gas_sensor.write = user_i2c_write;
    gas_sensor.delay_ms = user_delay_ms;
    gas_sensor.amb_temp = 25;


    int8_t rslt = BME680_OK;
    rslt = bme680_init(&gas_sensor);
    uint8_t set_required_settings;
    gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

    for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-t") == 0) {
                gas_sensor.tph_sett.os_temp = BME680_OS_8X;
                
            }
            if (strcmp(argv[i], "-p") == 0) {
                gas_sensor.tph_sett.os_pres = BME680_OS_4X;
                
            }
            if (strcmp(argv[i], "-h") == 0) {
                gas_sensor.tph_sett.os_hum = BME680_OS_2X;
                
            }
            if (strcmp(argv[i], "-g") == 0) {
                gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
                /* Create a ramp heat waveform in 3 steps */
                gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
                gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */
                
            } 
            if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "") == 0)  {
                printf("Usage: sense [-t -p -h -g]\n");
                printf("-t: Temperature oversampling\n");
                printf("-p: Pressure oversampling\n");
                printf("-h: Humidity oversampling\n");
                printf("-g: Enable gas measurement\n");
                return 0;
            }
        }


    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    gas_sensor.power_mode = BME680_FORCED_MODE;

    /* Set the required sensor settings needed */
    set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL | BME680_GAS_SENSOR_SEL;

    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);

    /* Set the power mode */
    rslt = bme680_set_sensor_mode(&gas_sensor);

    /* Get the total measurement duration so as to sleep or wait till the
    * measurement is complete */
    uint16_t meas_period;
    bme680_get_profile_dur(&meas_period, &gas_sensor);

    struct bme680_field_data data;


    while(1)
    {
        user_delay_ms(meas_period); /* Delay till the measurement is ready */

            rslt = bme680_get_sensor_data(&data, &gas_sensor);

            printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ", data.temperature / 100.0f,
                data.pressure / 100.0f, data.humidity / 1000.0f );
            /* Avoid using measurements from an unstable heating setup */
            if(data.status & BME680_GASM_VALID_MSK)
                printf(", G: %d ohms", data.gas_resistance);

            printf("\r\n");

            /* Trigger the next measurement if you would like to read data out continuously */
            if (gas_sensor.power_mode == BME680_FORCED_MODE) {
                rslt = bme680_set_sensor_mode(&gas_sensor);
            }
    }
    return 0;
}



