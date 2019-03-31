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
#include "globals.h"
#include "timeprocessing.h"
#include "frequencyprocessing.h"
#include </usr/local/include/fftw3.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CLIENT_SOCK_PATH "/tmp/power_data.sock" //This is where you send data, you are the client to the python server.
#define SERVER_SOCK_PATH "/tmp/gui_control.sock" //This is where you receive data, you are the server to the python client.
#define BUFF_SIZE 16 //The size of the rf_data struct - used for transmitting the structure
#define TEST_RUN_LENGTH 60 //Arbitrary test run length

#define PI 3.14159265 //For quick demos sake

struct rf_data{
    double data;
    double angle;
};

int main(void){//int argc, char **argv){
    struct signal data;
    struct signal real_data;
    struct signal imag_data;
    struct signal psdx;
    struct max_values val;
    double buf[FFT_SIZE] = {0};
    double Power;
    FILE *dataIn;
    //uint32_t raw_adc_data[WINDOW_SIZE];
    struct rf_data test_data;
    int len_client;
    int client_sock;
    int rc;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    /* Allocate the client structure from the kernel */
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock < 0){
        printf("SOCKET ERROR: %d\n", client_sock);
        exit(1);
    }
    /* Configure the client socket */
    client_sockaddr.sun_family = AF_UNIX;
    strcpy(client_sockaddr.sun_path, CLIENT_SOCK_PATH);
    len_client = sizeof(client_sockaddr);
    /* Connect to the python server with our client socket */
    rc = connect(client_sock, (struct sockaddr *) &client_sockaddr, len_client);
    if (rc == -1){
        printf("CONNECT ERROR = %d\n", rc);
        close(client_sock);
        exit(1);
    }

    /* FOR TEST PURPOSES */

    dataIn = fopen("testcode/testData.bin", "rb");
    if(!dataIn){
        printf("Cannot open file\n\r");
        return -1;
    }
    uint16_t numFiles = 37;
    uint16_t fileSize = 2000;
    uint32_t totalSize = numFiles * fileSize;
    double inputArray[totalSize];
    for(uint32_t i=0; i<totalSize; i++){
        fread(&inputArray[i], sizeof(double), 1, dataIn);
    }
    for(uint16_t k=0; k<numFiles; k++){
        for(uint32_t i = 0; i<fileSize; i++){
            data.values[i] = inputArray[k*fileSize + i];
        }
        data.length = fileSize;

        /* END TEST STUFF */

        /*
        system("/bin/cat /dev/hsdk > /tmp/sample.bin");
        sleep(1);
        printf("Successfully sampled ADC\r\n");

        dataIn = fopen("/tmp/sample.bin", "rb");
        if(!dataIn){
            printf("Cannot open file\n\r");
            return -1;
        }

        for(uint32_t i=0; i<WINDOW_SIZE; i++){
            fread(&raw_adc_data[i], sizeof(uint32_t), 1, dataIn);
        }

        fclose(dataIn);
        */

        //data = reorderData(raw_adc_data, WINDOW_SIZE);
        //data = decimateData(data);
        //data = windowData(data);
        //data = zeroPad(data);
        double *in;
        double *out;
        fftw_plan plan;

        in = (double*) fftw_malloc(sizeof(double) * data.length);
        out = (double*) fftw_malloc(sizeof(double) * data.length);
        plan = fftw_plan_r2r_1d(data.length, in, out, FFTW_R2HC, FFTW_MEASURE);

        for(uint32_t i=0; i<data.length; i++){
            in[i] = data.values[i];
        }

        fftw_execute(plan);

        imag_data.values[0] = 0;
        for(uint32_t i=0; i<data.length/2+1; i++){
            real_data.values[i] = out[i];
        }
        uint32_t j=data.length-1;
        for(uint32_t i=1; i<data.length/2; i++){
            imag_data.values[i] = out[j];
            j--;
        }
        imag_data.values[0] = 0;

        real_data.length = data.length/2;
        imag_data.length = data.length/2;

        real_data.fs = imag_data.fs = 1000;//6173300; //~18 MHz is roughly our sampling frequency, this is divided by 3 becasue we have decimated by 3

        //when using this in real application save plan through fftw_export_wisdom_to_filename(const char *filename);
        fftw_destroy_plan(plan);
        fftw_free(in); fftw_free(out);

        psdx = calculateMagSquared(real_data, imag_data);
        psdx = filter(psdx);
        val = findPeak(psdx);
        //interpolate(psdx, val, buf);
        Power = calculatePower(psdx.values, psdx.length);
        test_data.data = Power;
        //int intvar = 0;
        //if (sscanf (argv[1], "%i", &intvar) != 1) {
        //    fprintf(stderr, "error - not an integer");
        //}
        double angle = (double)k*180/36;
        test_data.angle = angle;//intvar;
        printf("Test Data Data is: %f, Test Data Angle is: %f\r\n", test_data.data, angle);
        /* Send the data over the client socket to the server */
        printf("Sending data...\n");
        rc = send(client_sock, &test_data, BUFF_SIZE, 0);
        if (rc == -1) {
            printf("SEND ERROR: %d", rc);
            close(client_sock);
            exit(1);
        }

        printf("psdx fs: %d\n\r", psdx.fs);
        printf("Power output: %lf\n\r\n\r", Power);
    }

    return 0;
}
