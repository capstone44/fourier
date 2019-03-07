#ifndef FREQUENCYPROCESSING_H_    /* Include guard */
#define FREQUENCYPROCESSING_H_

struct signal keepPostiveFreq(struct signal data);


double calculateMagSquared(float real_data[], float imag_data, uint16_t N, uint32_t fs);


double findPeak();


double interpolate();


double calculatePower();

#endif  // FREQUENCYPROCESSING_H_
