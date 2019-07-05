#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

void main (void) {
    int file;
    char filename[40];
    int addr = 0x4C; // The I2C address of the DAC

    sprintf(filename,"/dev/i2c-1");
    if ((file = open(filename,O_RDWR)) < 0) {
        printf("Failed to open the bus.");
        exit(1);
    }

    if (ioctl(file,I2C_SLAVE,addr) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
    // 172 is 86*2, +1 is for the control byte, and +10 is for optimization
    // slack.  We only write 172+1 to the DAC
    char buf[172 + 1 + 10] = {0};
    buf[0] = 0x16; // Register we are writing to
    int k = 1;
    for(int i = 0x55; i <= 0xAB; i++){
            buf[k] = i;
            k += 2;
    }

    while (1){
        write(file,buf,172 + 1);
    }
}
