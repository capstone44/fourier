#ifndef GLOBALS_H_   /* Include guard */
#define GLOBALS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define WINDOW_SIZE 50000
#define DECIMATION_FACTOR 3
#define DECIMATED_SIZE WINDOW_SIZE/DECIMATION_FACTOR
#define FIR_FILTER_TAP_COUNT 121
#define ADC_LENGTH 10

const char ADC1_GPIO[] = {9,25,10,22,27,17,18,15,14,24};
const char ADC2_GPIO[] = {20,26,16,19,13,12,7,8,11,21};

extern float window_weights[WINDOW_SIZE];

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
    float actual_max_value;
    float actual_max_frequency;
    float left;
    float right;
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
