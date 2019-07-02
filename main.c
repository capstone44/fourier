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
#define BYTES 4
#define ADC_ADDR 0x48
#define MY_BASE 2222
#define ADC_CHAN 3
#define ADC_SAMP 6

#define NUMTAPS 301
#define BANDS 2
#define BAND_EDGES 4

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
        data.values[i] = analogRead(MY_BASE + 3);
    }

    //Perform time processing
    //data = reorderData(raw_adc_data, WINDOW_SIZE);
    //data = decimateData(data);
    //data = windowData(data);

    data.fs = fs;
    data.length = WINDOW_SIZE;

    double *in;
    double *out;

    fftw_plan plan;

    in = (double *)fftw_malloc(sizeof(double) * 4096);
    out = (double *)fftw_malloc(sizeof(double) * 4096);

    plan = fftw_plan_r2r_1d(4096, in, out, FFTW_R2HC, FFTW_MEASURE);

    for (uint32_t i = 0; i < 4096; i++)
    {
        if(i < data.length)
        {
            in[i] = data.values[i];
        }
        else
        {
            in[i] = 0;
        }
    }

    fftw_execute(plan);

    imag_data.values[0] = 0;
    for (uint32_t i = 0; i < data.length / 2 + 1; i++)
    {
        real_data.values[i] = out[i];
    }
    uint32_t j = data.length - 1;
    for (uint32_t i = 1; i < data.length / 2; i++)
    {
        imag_data.values[i] = out[j];
        j--;
    }
    imag_data.values[0] = 0;

    real_data.length = data.length / 2;
    imag_data.length = data.length / 2;

    real_data.fs = imag_data.fs = fs;

    //when using this in real application save plan through fftw_export_wisdom_to_filename(const char *filename);
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    psdx = calculateMagSquared(real_data, imag_data);
    //psdx = filter_default(psdx);

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

    //printf("psdx fs: %d\n\r", psdx.fs);
    //printf("Power output: %lf\n\r\n\r", Power);

return 0;
}
