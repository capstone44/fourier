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

#define CLIENT_SOCK_PATH "/tmp/power_data.sock" //This is where you send data, you are the client to the python server.
#define SERVER_SOCK_PATH "/tmp/gui_control.sock" //This is where you receive data, you are the server to the python client.
#define BUFF_SIZE 6 //The size of the rf_data struct - used for transmitting the structure
#define TEST_RUN_LENGTH 60 //Arbitrary test run length

#define PI 3.14159265 //For quick demos sake

struct rf_data{
    double data;
    int16_t angle;
};


int main(void){
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
    double power = 0;
    int counter = 0;

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

    return 0;
}

