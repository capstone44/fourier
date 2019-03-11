#ifndef TIMEPROCESSING_H_    /* Include guard */
#define TIMEPROCESSING_H_

void GetV2(float values[], uint32_t N, float *M);

void GetV1(float values[], uint32_t N, float *M2);

/**********************************************/
/* The output of the ADC does not have the    */
/* data in the correct order, so before and   */
/* processing can be done, the order must     */
/* be corrected.                              */
/**********************************************/
struct signal reorderData(struct signal data);

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
/* We are sampling way above what the beat    */
/* frequency will be, and while this allows   */
/* for future extension of the system, right  */
/* now we want to increase computational      */
/* efficiency by only keeping 1/3 of our      */
/* sampled data, and to do this we need to    */
/* first anti-alias filter the data.          */
/**********************************************/
double antiAliasFilter();

/**********************************************/
/* Now that the data has been passed through  */
/* the anti-alias filter, we will decimate    */
/* by three (keep every third sample).        */
/**********************************************/
double decimateData();


void testCode(struct signal data);


#endif  // TIMEPROCESSING_H_
