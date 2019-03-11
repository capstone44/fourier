#include <stdio.h>
#include <stdlib.h>
#include "timeprocessing.h"
#include "globals.h"

void GetV2(float values[], uint32_t N, float *M){
    int32_t val;
    float tmp, SignalZero;
    uint32_t shift;
    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in a signed int.       */
    for(uint32_t i=0; i<N; i++){
        val = int(values[i]);
        SignalZero = 0;
        tmp = 0;
        shift = 0;
        /* Iterate over the GPIO pin mask array for */
        /* this ADC, create a shift variable that   */
        /* creates the mask by shifting the 1 the   */
        /* amount of spaces dictated by the mask    */
        /* array. Perform the bitwise and with the  */
        /* mask and then divide by the value of the */
        /* mask array, find the current value by    */
        /* raising 2 to the power of the previous   */
        /* value - 1 (done by shift operation), add */
        /* to previous value.                       */
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            shift = ADC2_GPIO[k];
            tmp = 1 << shift;
            tmp = val & tmp;
            tmp /= shift;
            tmp = 1 << tmp;
            SignalZero += tmp;
        }
        /* After iterating through the mask and        */
        /* summing the values, store in current index. */
        M[i] = SignalZero;
    }
}

void GetV1(float values[], uint32_t N, float *M2){
    int32_t val;
    float tmp, SignalZero;
    uint32_t shift;
    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in a signed int.       */
    for(uint32_t i=0; i<N; i++){
        val = int(values[i]);
        SignalZero = 0;
        tmp = 0;
        shift = 0;
        /* Iterate over the GPIO pin mask array for */
        /* this ADC, create a shift variable that   */
        /* creates the mask by shifting the 1 the   */
        /* amount of spaces dictated by the mask    */
        /* array. Perform the bitwise and with the  */
        /* mask and then divide by the value of the */
        /* mask array, find the current value by    */
        /* raising 2 to the power of the previous   */
        /* value - 1 (done by shift operation), add */
        /* to previous value.                       */
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            shift = ADC1_GPIO[k];
            tmp = 1 << shift;
            tmp = val & tmp;
            tmp /= shift;
            SignalZero += 1 << tmp;
        }
        /* After iterating through the mask and        */
        /* summing the values, store in current index. */
        M2[i] = SignalZero;
    }
}

struct signal reorderData(struct signal data){
    /* Throw out last data point, as it it junk. */
    /* Create pointers to data arrays that will  */
    /* be used to reorder the input data.        */
    data.length--;
    float M[data.length];
    float M2[data.length];
    int i = 0, j = 0, k = 0;

    /* These two functions will perform the intial */
    /* calculations for reordering the data and    */
    /* return the values in the buffer arrays M,M2 */
    GetV2(data.values, data.length, M);
    GetV1(data.values, data.length, M2);

    /* Theese three while loops will interleave    */
    /* the data stored in the two buffers while    */
    /* both buffers still have data, then it will  */
    /* append the remaining data, if any, from the */
    /* buffer that is not empty.                   */
    while(i<sizeof(M) && j<sizeof(M2)){
        data.values[k++] = M[i++];
        data.values[k++] = M2[j++];
    }
    while(i<sizeof(M))
        data.values[k++] = M[i++];
    while(j<sizeof(M2))
        data.values[k++] = M2[j++];
    
    return data;
}

/* windowData will accept a a struture with    */
/* an array of ADC values and will multiply    */
/* each value with a corresponding window      */
/* weight found in the globals.c file. The     */
/* struct with the data array will  then be    */
/* returned to the main function.              */
struct signal windowData(struct signal data){
    for(uint32_t i=0; i<data.length; i++)
        data.values[i] = data.values[i] * window_weights[i];
    return data;
}

float antiAliasFilter(){
    float y = 0; //value for y queue
    for(uint32_t i = 0; i < FIR_COEF_COUNT; i++) // Iterates through the size of the x queue.
    {
        y += queue_readElementAt(&xQueue, FIR_COEF_COUNT - i - OFFSET)*firCoefficients[i]; // Convolves x queue with the filter coefficients.
    }
    queue_overwritePush(&yQueue, y); // Add the computed value to the y queue.
    return y; //Return the value as a double.
}

static uint16_t firDecimationCount = 0;
double decimateData(){
    if (firDecimationCount == DECIMATION_FACTOR - 1)
    {
        antiAliasFilter(); // Run antialias filter.
        firDecimationCount = 0; // Reset decimation count.
        return true; // Ran the filter.
    }
    firDecimationCount++; // Increment the decimation count each time the filter fuction is called.
    return false; // Didn't run the filter.
}

void testCode(struct signal data){
    /* Call whichever function is under test */
    data = reorderData(data);


    /* Print data to text file to compare with Matlab */
    FILE *dataOut;
    dataOut = fopen("1mhz_out.txt","wb");
    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++)
        fprintf(dataOut, "%0.3lf\n", data.values[i]);

    fclose(dataOut);
}
