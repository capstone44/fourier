#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include </usr/local/include/fftw3.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "globals.h"
#include "timeprocessing.h"
#include "frequencyprocessing.h"
#include <liquid/liquid.h>

int main(){

    FILE *dataIn;
    uint32_t raw_adc_data[WINDOW_SIZE];
    struct signal data;
    struct signal real_data;
    struct signal imag_data;
    struct signal psdx;
    struct max_values val;
    double buf[FFT_SIZE];

    dataIn = fopen("testcode/ADCTesting/750k.bin", "rb");
    if(!dataIn)
    {
        printf("Cannot open file\n\r");
        return -1;
    }
    for(uint32_t i=0; i<WINDOW_SIZE; i++)
    {
        fread(&raw_adc_data[i], sizeof(uint32_t), 1, dataIn);
    }
    fclose(dataIn);

    data = reorderData(raw_adc_data, WINDOW_SIZE);
    data = decimateData(data);
    data = windowData(data);

    double *in;

}
