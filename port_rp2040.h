#include "ad5593r/common/common.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

extern HAL_Sleep sleep_nano;
extern HAL_I2C hal_i2c;
extern HAL_PinOut enable_a0;

extern void sleep_nano_pico(int64_t nanoseconds);
extern error i2c_tx_wrapper(uint16 i2c_address, byte *write_data, isize wlen, byte *read_data, isize rlen);
extern error rp2040_setup(int a0);
extern error ad5593r_config();
extern error read_ADC_data(int num_reads, int refresh_time_secs, int channel);
