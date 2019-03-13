#ifndef TIMEPROCESSING_H_    /* Include guard */
#define TIMEPROCESSING_H_

#include "globals.h"

extern const char ADC1_GPIO[];
extern const char ADC2_GPIO[];

/**********************************************/
/* The output of the ADC does not have the    */
/* data in the correct order, so before and   */
/* processing can be done, the order must     */
/* be corrected. This function calls the      */
/* above functions and then interleaves their */
/* outputs.                                   */
/**********************************************/
struct signal reorderData(uint32_t raw_adc_data[], uint32_t N);

/**********************************************/
/* In order to remove multipath with a higher */
/* resolution than our ADC affords we will    */
/* use a parabolic interpolation later on.    */
/* By windowing the data, we can take         */
/* advantage of the peak smearing to get      */
/* better points for our parabolic fit.       */
/**********************************************/
struct signal windowData(struct signal data);

/**********************************************/
/* Now that the data has been passed through  */
/* the anti-alias filter, we will decimate    */
/* by three (keep every third sample).        */
/**********************************************/
struct signal decimateData(struct signal data);

/**********************************************/
/* This function will run one of the above    */
/* functions and then print the new data to   */
/* an output file to compare with the         */
/* expected values in Matlab, this will help  */
/* validate the data and performance of the   */
/* function under test.                       */
/**********************************************/

/*************************************************************/
/* This pound define is for testing convenience. In order to */
/* test certain functions set its value to the following:    */
/* reorderData()     : 1                                     */
/* decimateData()    : 2                                     */
/* windowData()      : 3                                     */
/*************************************************************/
#define TEST_FUNCTION 2

#if TEST_FUNCTION == 1
void testCode(uint32_t raw_adc_data[], uint32_t N);
#else
void testCode(struct signal data);
#endif

/* Need to fill this array with values from Matlab.    */
/* May want to create a separate file for this         */
/* because there will be 16,385 floating point values. */
const float window_weights[DECIMATED_SIZE];


#endif  // TIMEPROCESSING_H_
