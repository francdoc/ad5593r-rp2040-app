#include <stdio.h>

#include "ad5593r/ad5593r.h"

#include "port_rp2040.h"

#define microseconds (1000)
#define milliseconds (1000 * microseconds)
#define seconds (1000LL * milliseconds)

#define DELAY (0.025) // seconds

#define SDA 10
#define SCL 11
#define i2c i2c1_inst

HAL_Sleep sleep_nano;
HAL_I2C hal_i2c;
HAL_PinOut enable_a0;

AD5593R i2c_device;

// rp2040 port

void sleep_nano_pico(int64_t nanoseconds)
{
    if (nanoseconds < 0)
    {
        panic("negative time value");
    }
    sleep_us((uint64_t)(nanoseconds / 1000));
}

void set_pin_a0(bool state)
{
    gpio_put(i2c_device._a0, state); // port to rp2040
}

error i2c_tx_wrapper(uint16 i2c_address, byte *write_data, isize wlen, byte *read_data, isize rlen)
{
    bool hasWriteData = write_data != NULL && wlen > 0;
    bool hasReadData = read_data != NULL && rlen > 0;

    if (hasWriteData)
    {
        int result = i2c_write_blocking(&i2c, i2c_address, write_data, wlen, hasReadData);
        if (result < 0)
        {
            return -1; // Indicate an error
        }
    }

    if (hasReadData)
    {
        int result = i2c_read_blocking(&i2c, i2c_address, read_data, rlen, false);
        if (result < 0)
        {
            return -1; // Indicate an error
        }
    }

    return 0; // Success
}

error rp2040_setup(int a0)
{
    sleep_nano = &sleep_nano_pico;
    enable_a0 = &set_pin_a0;
    hal_i2c.tx = &i2c_tx_wrapper;

    // Sleep a tad bit to let connected PC reach USB.
    sleep_nano(5 * seconds);

    i2c_device = new_AD5593R(enable_a0, a0, hal_i2c);

    // Initialize the GPIO configuration arrays
    if (i2c_device._a0 > -1)
    {
        gpio_init(i2c_device._a0);
        gpio_set_dir(i2c_device._a0, GPIO_OUT);
        enable_a0(true);
    }

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico) -> hw pins 6 and 7.
    i2c_init(&i2c, 100 * 1000);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_set_function(SDA, GPIO_FUNC_I2C);

    gpio_pull_up(SCL);
    gpio_pull_up(SDA);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(SDA, SCL, GPIO_FUNC_I2C));

    printf("rp2040 setup OK.\n");

    sleep_nano(DELAY * seconds);

    return 0;
}

error ad5593r_config()
{
    bool hw_DACs[8] = {1, 1, 1, 1, 0, 0, 0, 0};
    bool hw_ADCs[8] = {0, 0, 0, 0, 1, 1, 1, 1};

    error err;

    err = AD5593R_enable_internal_Vref(&i2c_device);
    if (err != 0)
    {
        printf("Error in enabling internal Vref.\n");
        return -1;
    }
    printf("AD5593R_enable_internal_Vref.\n");

    sleep_nano(DELAY * seconds);

    // Set DAC max voltage to 2xVref
    err = AD5593R_set_DAC_max_2x_Vref(&i2c_device);
    if (err != 0)
    {
        printf("Error in setting DAC max to 2x Vref.\n");
        return -1;
    }
    printf("AD5593R_set_DAC_max_2x_Vref.\n");

    sleep_nano(DELAY * seconds);

    // Configure DAC channels based on the values in hw_DACs array
    err = AD5593R_configure_DACs(&i2c_device, hw_DACs, _num_of_channels);
    if (err != 0)
    {
        printf("Error in configuring DACs.\n");
        return -1;
    }
    printf("AD5593R_configure_DACs.\n");

    sleep_nano(DELAY * seconds);

    err = AD5593R_write_DAC(&i2c_device, 0, 1.0); // channel 0
    if (err != 0)
    {
        printf("Error in writing to DAC channel 0.\n");
        return -1;
    }
    err = AD5593R_write_DAC(&i2c_device, 1, 2.0); // channel 1
    if (err != 0)
    {
        printf("Error in writing to DAC channel 1.\n");
        return -1;
    }

    err = AD5593R_write_DAC(&i2c_device, 2, 3.0); // channel 2
    if (err != 0)
    {
        printf("Error in writing to DAC channel 3.\n");
        return -1;
    }

    err = AD5593R_write_DAC(&i2c_device, 3, 4.0); // channel 3
    if (err != 0)
    {
        printf("Error in writing to DAC channel 4.\n");
        return -1;
    }

    printf("AD5593R_write_DACs.\n");

    sleep_nano(DELAY * seconds);

    // Set ADC max voltage to 2xVref
    err = AD5593R_set_ADC_max_2x_Vref(&i2c_device);
    if (err != 0)
    {
        printf("Error in setting ADC max to 2x Vref.\n");
        return -1;
    }
    printf("AD5593R_set_ADC_max_2x_Vref.\n");

    sleep_nano(DELAY * seconds);

    // Configure ADC channels based on the values in hw_ADCs array
    err = AD5593R_configure_ADCs(&i2c_device, hw_ADCs, _num_of_channels);
    if (err != 0)
    {
        printf("Error in configuring ADCs.\n");
        return -1;
    }
    printf("AD5593R_configure_ADCs.\n");

    sleep_nano(DELAY * seconds);

    return 0;
}

error read_ADC_data(int num_reads, int refresh_time_ns, int channel)
{
    for (int i = 0; i < num_reads; i++)
    {
        float adc_value = AD5593R_read_ADC(&i2c_device, channel); // Read from channel
        if (adc_value < 0)
        {
            printf("Error in reading from ADC channel %d.\n", channel);
        }
        printf("ADC channel %d reads: %.4f Volts\n", channel, adc_value);

        sleep_nano(refresh_time_ns);
    }

    return 0;
}