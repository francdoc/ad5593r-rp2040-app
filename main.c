#include "ad5593r/common/common.h"
#include "port_rp2040.h"

// Run the next command to check prints:
// $ minicom -b 115200 -o -D /dev/ttyACM0

int main(void)
{
    // Initialize standard input/output
    stdio_init_all();

    error err;

    int a0 = -1;
    err = rp2040_setup(a0);
    if (err != 0)
    {
        printf("Error in setup.\n");
        return -1;
    }

    printf("Test program init.\n");

    err = ad5593r_config();
    if (err != 0)
    {
        printf("Error in ad5993r config.\n");
        return -1;
    }

    int num_reads = 1500;
    int refresh_time_secs = 500000000;
    int channel = 4;

    err = read_ADC_data(num_reads, refresh_time_secs, channel);
    if (err != 0)
    {
        printf("Error in ad5993r ADC readout.\n");
        return -1;
    }

    return 0;
}