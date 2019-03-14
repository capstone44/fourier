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

#define CLIENT_SOCK_PATH "/tmp/power_data.sock" //This is where you send data, you are the client to the python server.
#define SERVER_SOCK_PATH "/tmp/gui_control.sock" //This is where you receive data, you are the server to the python client.
#define BUFF_SIZE 6 //The size of the rf_data struct - used for transmitting the structure
#define TEST_RUN_LENGTH 60 //Arbitrary test run length

#define PI 3.14159265 //For quick demos sake

struct rf_data{
    float data;
    int16_t angle;
};

/****************************************************************/
/* To run normally, set TEST to 0                               */
/* To run a test on time domain functions, set TEST to 1        */
/* To run a test on frequency domain functions, set TEST to 2   */
/****************************************************************/
#define TEST 2

int main(void){
    #if !TEST
    /*
     * Set up variables for each of the client and server sockets
     */
    int len_client;
    int client_sock;
    int rc,angle;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    bool run = true;
    /*
     * Sets up the test data and structure
     */
    struct rf_data test_data;
    uint32_t adc_read[WINDOW_SIZE];
    struct signal data;
    struct signal real_data;
    struct signal imag_data;
    struct signal psdx;
    struct max_values values_for_interpolate;
    float buf[DECIMATED_SIZE] = {0};
    float power = 0;
    int counter = 0;

    /*
     * Initialize struct values to zero
     */

    data.length = data.fs = data.delta_f = 0;
    real_data.length = real_data.fs = real_data.delta_f = 0;
    imag_data.length = imag_data.fs = imag_data.delta_f = 0;
    psdx.length = psdx.fs = psdx.delta_f = 0;

    /*
     * Zero the socket structures
     */
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

    while (run){

        /* TODO: The following power compute functions have not been debugged
         * and are simply placeholders while I flesh out my ideas
         */
        data = reorderData(adc_read,WINDOW_SIZE);
        windowData(data);
        decimateData();

        // Send data to fft function

        real_data = keepPositiveFreq(real_data);
        imag_data = keepPositiveFreq(imag_data);
        psdx = calculateMagSquared(real_data, imag_data);
        values_for_interpolate = findPeak(psdx);
        interpolate(psdx, values_for_interpolate, buf);
        power = calculatePower(buf, psdx.length, psdx.delta_f);

        if(counter < TEST_RUN_LENGTH){
            printf("Test Run Length is: %d, Counter is at: %d\r\n", TEST_RUN_LENGTH, counter);
            angle = (360 / TEST_RUN_LENGTH) * counter;
            test_data.data = fabs(5*(sinf(angle * (PI/180))));
            test_data.angle = angle;
            printf("Test Data Data is: %f, Test Data Angle is: %d\r\n", test_data.data, test_data.angle);
            /* Send the data over the client socket to the server */
            printf("Sending data...\n");
            rc = send(client_sock, &test_data, BUFF_SIZE, 0);
            if (rc == -1) {
                printf("SEND ERROR: %d", rc);
                close(client_sock);
                exit(1);
            }
            else {
                printf("Data sent!\n");
            }
        }
        else{
            run = false;
        }
        counter++;
    }
    close(client_sock);

    #else

    FILE *dataInReal, *dataInImag;

    #if TEST == 1

    uint32_t raw_adc_data[WINDOW_SIZE];

    dataIn = fopen("testcode/ADCTesting/750k.bin", "rb");
    if(!dataIn){
        printf("Cannot open file\n\r");
        return -1;
    }

    for(uint32_t i=0; i<WINDOW_SIZE; i++){
        fread(&raw_adc_data[i], sizeof(uint32_t), 1, dataIn);
    }

    fclose(dataIn);

    struct signal data = reorderData(raw_adc_data, WINDOW_SIZE);
    data = decimateData(data);
    testCodeTime(data);

    #else

    struct signal real_data, imag_data;
    real_data.length = imag_data.length = FFT_SIZE;
    real_data.fs = imag_data.fs = 6173300;

    dataInReal = fopen("testcode/ADCTesting/750k_fft_real.bin", "rb");
    dataInImag = fopen("testcode/ADCTesting/750k_fft_imag.bin", "rb");
    if(!dataInReal){
        printf("Cannot open file\n\r");
        return -1;
    }
    if(!dataInImag){
        printf("Cannot open file\n\r");
        return -2;
    }

    for(uint32_t i=0; i<FFT_SIZE; i++){
        fread(&real_data.values[i], sizeof(float), 1, dataInReal);
        fread(&imag_data.values[i], sizeof(float), 1, dataInImag);
    }

    fclose(dataInReal);
    fclose(dataInImag);

    real_data = keepPositiveFreq(real_data);
    imag_data = keepPositiveFreq(imag_data);
    testCodeFreq(real_data, imag_data);

    #endif

    #endif
    return 0;
}
