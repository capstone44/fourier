#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void main (void) {
    int file;
    char filename[40];
    int addr = 0x4C;        // The I2C address of the ADC

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        exit(1);
    }

    if (ioctl(file,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    char buf[10] = {0};

    buf[0] = 0x16; // Register we are writing to
    for(int i = 0x55F; i < 0xABF; i++){
        buf[1] = (i & 0xff0) >> 4;
        buf[2] = (buf[1] & 0xf) << 4;
        if (write(file,buf,3) != 1) {
            /* ERROR HANDLING: i2c transaction failed */
            printf("Failed to write to the i2c bus.\n");
            printf("\n\n");
        }
    }
}
