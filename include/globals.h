#ifndef GLOBALS_H_   /* Include guard */
#define GLOBALS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define WINDOW_SIZE 50000       //Size of data coming in from ADC after interleaving
#define DECIMATION_FACTOR 3
#define DECIMATED_SIZE WINDOW_SIZE/DECIMATION_FACTOR
#define ADC_LENGTH 10
#define FFT_SIZE 32768
#define LOG_FFT_SIZE 15

/**************************************************************/
/* Struct that holds all of the info for data being processed */
/* including its current length, sampling rate, data values   */
/* frequency values, and change in frequency per index        */
/**************************************************************/
typedef struct signal
{
    uint32_t length;
    uint32_t fs;
    double values[WINDOW_SIZE];
    double frequencies[WINDOW_SIZE];
    double delta_f;
}signal;

typedef struct fft_signal
{
    signal real_signal;
    signal imag_signal;
}fft_signal;

/**************************************************************/
/* Struct that holds all of the info needed for interpolation */
/* including the index to the left of the interpolated max    */
/* value, the index to the right of the interpolated max      */
/* value, the interpolated max value, the interpolated        */
/* frequency for the max value, the actual value in the index */
/* left of the max value index, the actual value in the index */
/* right of the max value.                                    */
/**************************************************************/
typedef struct max_values
{
    uint32_t left_index;
    uint32_t right_index;
    double actual_max_value;
    double actual_max_frequency;
    double left_value;
    double right_value;
}max_values;

/* THE FUNCTION BELOW IS NOT ACTUALLY USED, BUT WILL BE KEPT FOR FUTURE EXPANSION IF NEEDED */

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
