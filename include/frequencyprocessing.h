#ifndef FREQUENCYPROCESSING_H_    /* Include guard */
#define FREQUENCYPROCESSING_H_

#include "globals.h"

struct signal keepPositiveFreq(struct signal data);


struct signal calculateMagSquared(struct signal real_data, struct signal imag_data);


struct max_values findPeak(struct signal psdx);


void interpolate(struct signal psdx, struct max_values val, float *buf);


float calculatePower(float *buf, uint32_t N, float delta_f);

/* Need to fill this array with values from Matlab.    */
/* May want to create a separate file for this         */
/* because there will be 16,385 floating point values. */
const float window_weights[DECIMATED_SIZE];

#endif  // FREQUENCYPROCESSING_H_