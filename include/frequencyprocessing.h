#ifndef FREQUENCYPROCESSING_H_    /* Include guard */
#define FREQUENCYPROCESSING_H_

struct signal keepPositiveFreq(struct signal data);


struct signal calculateMagSquared(struct signal real_data, struct signal imag_data);


struct max_values findPeak(struct signal psdx);


void interpolate(struct signal psdx, struct max_values val, float *buf);


float calculatePower(float *buf, uint32_t N, float delta_f);

#endif  // FREQUENCYPROCESSING_H_
