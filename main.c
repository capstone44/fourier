/**************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *************************************************************************/
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
#include <linux/i2c-dev.h>
#include <wiringPi.h>
//#include <ads1115.h>
#include "ads.h"
#include <liquid/liquid.h>
#include "globals.h"
#include "timeprocessing.h"
#include "frequencyprocessing.h"

#define CLIENT_SOCK_PATH "/tmp/power_data.sock"  //This is where you send data, you are the client to the python server.
#define SERVER_SOCK_PATH "/tmp/gui_control.sock" //This is where you receive data, you are the server to the python client.
#define BUFF_SIZE 16                             //The size of the rf_data struct - used for transmitting the structure
#define TEST_RUN_LENGTH 60                       //Arbitrary test run length

#define NANO_SECOND 1e-9

#define ADC_ADDR 0x48
#define MY_BASE 2222


struct rf_data
{
    double data;
    double angle;
};

int main(void)
{
    // Create variable to handle data processing
    struct signal data;
    struct signal real_data;
    struct signal imag_data;
    struct signal psdx;
    struct max_values val;
    double buf[FFT_SIZE] = {0};
    double Power;
    //FILE *dataIn;
    //uint32_t raw_adc_data[WINDOW_SIZE];
    struct rf_data test_data;

    // Create variable to handle the UNIX socket
    int len_client;
    int client_sock;
    int rc;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;

    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

    /* Allocate the client structure from the kernel */
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0)
    {
        printf("SOCKET ERROR: %d\n", client_sock);
        exit(1);
    }
    /* Configure the client socket */
    client_sockaddr.sun_family = AF_UNIX;
    strcpy(client_sockaddr.sun_path, CLIENT_SOCK_PATH);
    len_client = sizeof(client_sockaddr);
    /* Connect to the python server with our client socket */
    rc = connect(client_sock, (struct sockaddr *)&client_sockaddr, len_client);
    if (rc == -1)
    {
        printf("CONNECT ERROR = %d\n", rc);
        close(client_sock);
        exit(1);
    }

    // Read sampled data
    /*
    system("/bin/cat /dev/hsdk > /tmp/sample.bin");
    sleep(1);
    printf("Successfully sampled ADC\r\n");
    int file_desc = open("/dev/hsdk",0);
    double sample_time = ioctl(file_desc,0,0);
    double fs = 1/(sample_time*NANO_SECOND)*WINDOW_SIZE;
    */
    double fs = 860;

    ads1115Setup(MY_BASE, ADC_ADDR);
    for(uint32_t i=0; i<WINDOW_SIZE; i++){
        data.values[i] = (double)analogRead(MY_BASE + 3);
//        printf("data[%d]: %g\n\r", i, data.values[i]);
    }

    data.fs = fs;
    data.length = WINDOW_SIZE;

    //Perform time processing
    //data = reorderData(raw_adc_data, WINDOW_SIZE);
    //data = decimateData(data);
//    data = windowData(data);

    float h[121];
    uint32_t n = 1001;
    uint32_t fc = 100;
    uint32_t fc2 = 300;
    uint32_t numbands = 2;
    float bands[4] = {0.00f, 0.10f, 0.25f, 0.50f};
    float des[2] = {1.0f,0.0f};
    float weights[2] = {1.0f, 1.0f};
    liquid_firdespm_btype btype = LIQUID_FIRDESPM_BANDPASS;
    liquid_firdespm_wtype wtype[2]= {LIQUID_FIRDESPM_FLATWEIGHT, LIQUID_FIRDESPM_FLATWEIGHT};

//    firdespm_run(n, numbands, bands, des, weights, wtype, btype, h);

    double *in;
    double *out;
//    double *in_h;
//    double *out_h;

    fftw_plan plan;
//    fftw_plan plan_h;

    in = (double *)fftw_malloc(sizeof(double) * FFT_SIZE);
    out = (double *)fftw_malloc(sizeof(double) * FFT_SIZE);
//    in_h = (double *)fftw_malloc(sizeof(double) * FFT_SIZE);
//    out_h = (double *)fftw_malloc(sizeof(double) * FFT_SIZE);

    plan = fftw_plan_r2r_1d(FFT_SIZE, in, out, FFTW_R2HC, FFTW_MEASURE);
//    plan_h = fftw_plan_r2r_1d(FFT_SIZE, in_h, out_h, FFTW_R2HC, FFTW_MEASURE);

    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        if(i < data.length)
        {
            in[i] = data.values[i];
        }
        else
       {
            in[i] = 0;
        }
        if(i < n){
//            in_h[i] = (double)h[i];
        }
        else{
//            in_h[i] = 0;
        }

    }

    fftw_execute(plan);
//    fftw_execute(plan_h);

//    double H_real[FFT_SIZE] = {0};
//    double H_imag[FFT_SIZE] = {0};

    imag_data.values[0] = 0;
//    H_imag[0] = 0;
    for (uint32_t i = 0; i < FFT_SIZE / 2 + 1; i++)
    {
        real_data.values[i] = out[i];
//        H_real[i] = out_h[i];
    }
    uint32_t j = FFT_SIZE - 1;
    for (uint32_t i = 1; i < FFT_SIZE / 2; i++)
    {
        imag_data.values[i] = out[j];
//        H_imag[i] = out_h[j];
        j--;
    }
    imag_data.values[0] = 0;
//    H_imag[0] = 0;

    real_data.length = FFT_SIZE / 2;
    imag_data.length = FFT_SIZE / 2;

    real_data.fs = imag_data.fs = fs;

    //when using this in real application save plan through fftw_export_wisdom_to_filename(const char *filename);
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
 //   fftw_destroy_plan(plan_h);
 //   fftw_free(in_h);
 //   fftw_free(out_h);

 /*   double H[FFT_SIZE] = {0};
    uint64_t scaler = (uint64_t) fs * FFT_SIZE * 2;

    for(uint32_t i=0; i<FFT_SIZE/2; i++)
    {
        if(i != 0 && i != FFT_SIZE/2)
        {
            H[i] = 2*(H_real[i]+H_imag[i])/scaler;
        }
        else
        {
            H[i] = (H_real[i] + H_imag[i])/scaler;
        }
    }

    for(uint32_t i=0; i<100; i++){
        printf("H[%d]: %g\n\r", i, H[i]);
    }
*/
    psdx = calculateMagSquared(real_data, imag_data);
    //psdx = filter(psdx, H);

    val = findPeak(psdx);
    interpolate(psdx, val, buf);
    Power = calculatePower(psdx.values, psdx.length);
    test_data.data = Power;
    //int intvar = 0;
    //if (sscanf (argv[1], "%i", &intvar) != 1) {
    //    fprintf(stderr, "error - not an integer");
    //}
    double angle = 0;//(double)k * 180 / 36;
    test_data.angle = angle; //intvar;
    printf("Test Data Data is: %f, Test Data Angle is: %f\r\n", test_data.data, angle);
    /* Send the data over the client socket to the server */
    printf("Sending data...\n");
    rc = send(client_sock, &test_data, BUFF_SIZE, 0);
    if (rc == -1)
    {
        printf("SEND ERROR: %d", rc);
        close(client_sock);
        exit(1);
    }

    printf("Max Freq: %g\n\r", val.actual_max_frequency);
    //printf("psdx fs: %d\n\r", psdx.fs);
    //printf("Power output: %lf\n\r\n\r", Power);

return 0;
}
