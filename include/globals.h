#ifndef GLOBALS_H_   /* Include guard */
#define GLOBALS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define WINDOW_SIZE 50000
#define DECIMATION_FACTOR 3
#define DECIMATED_SIZE WINDOW_SIZE/DECIMATION_FACTOR
#define ADC_LENGTH 10

typedef struct signal
{
    uint32_t length;
    uint32_t fs;
    float delta_f;
    float values[WINDOW_SIZE];
    float frequencies[WINDOW_SIZE];
}signal;

typedef struct max_values
{
    uint32_t left_index;
    uint32_t right_index;
    float actual_max_value;
    float actual_max_frequency;
    float left_value;
    float right_value;
}max_values;

/**********************************************/
/* The FFT code that our system uses can only */
/* handle lengths that are a power of 2,      */
/* and it's still faster anyways, so we need  */
/* to find out how many samples we have and   */
/* return the next power of 2 so that we can  */
/* zero-pad the data before sending it to the */
/* fft function. This function will also be   */
/* used anywhere else we need a power of 2.   */
/**********************************************/
int nextPow2(uint16_t N);

#endif  // GLOBALS_H_
