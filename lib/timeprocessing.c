#include "timeprocessing.h"

const char ADC1_GPIO[] = {9,25,10,22,27,17,18,15,14,24};
const char ADC2_GPIO[] = {20,26,16,19,13,12,7,8,11,21};

float * GetV2(uint32_t values[]){
    uint32_t val,tmp;
    float SignalZero;
    uint32_t shift;

    static float M[WINDOW_SIZE/2];

    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in an unsigned int.    */
    for(uint32_t i=0; i<WINDOW_SIZE/2; i++){
        val = values[i];
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
    return M;
}

float * GetV1(uint32_t values[]){
    int32_t val,tmp;
    float SignalZero;
    uint32_t shift;

    static float M2[WINDOW_SIZE/2];

    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in an unsigned int.    */
    for(uint32_t i=0; i<WINDOW_SIZE/2; i++){
        val = values[i];
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
        /* value (done by shift operation), add     */
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
    return M2;
}

struct signal reorderData(uint32_t raw_adc_data[], uint32_t N){
    signal data;
    /* N can either be the length of the final structure, or the length of raw_adc_data
     * The array initializers and size will either then be N/2, or 2*N, respectively
     */
    uint32_t N2 = N/2;
    //float* M;
    //float* M2;
    float M[N2];
    float M2[N2];

    //M = GetV2(raw_adc_data);
    //M2 = GetV1(raw_adc_data);

    uint32_t val, tmp1, tmp2;
    uint32_t shift, shift1, shift2;
    float SignalZero1, SignalZero2;

    for(uint32_t i=0; i<N2; i++){
        val = raw_adc_data[i];
        //printf("Value of raw_adc_data[i]: %ld Value of val: %ld\n\r", raw_adc_data[i], val);
        SignalZero1 = SignalZero2 = 0;

        for(uint8_t k=0; k<ADC_LENGTH; k++){
            /*shift = 1 << k;         //2**k

            shift1 = ADC2_GPIO[k];  //ADC2_GPIO[k]
            shift1 = 1 << shift1;   //2**ADC2_GPIO[k]
            tmp1 = val & shift1;    //val & 2**ADC2_GPIO[k]
            tmp1 /= shift1;         //(val&2**ADC2_GPIO[k])/2**ADC2_GPIO[k]
            tmp1 = shift * tmp1;    //2**k*((val&2**ADC2_GPIO[k])/2**ADC2_GPIO[k])
            SignalZero1 += 1 << tmp1;
            
            shift2 = ADC1_GPIO[k];
            shift2 = 1 << shift2;
            tmp2 = val & shift2;
            tmp2 /= shift2;
            tmp2 = shift * tmp2;
            SignalZero2 += 1 << tmp2;
            */

            SignalZero1 += (1<<k)*((val&(1<<ADC2_GPIO[k])/(1<<ADC2_GPIO[k])));
            SignalZero2 += (1<<k)*((val&(1<<ADC1_GPIO[k])/(1<<ADC1_GPIO[k])));
        }
        M[i] = SignalZero1;
        M2[i] = SignalZero2;
    }

    /*for(uint32_t i=0; i<N2; i++){
        printf("Value of M at i: %d is %f\n", i, M[i]);
    }*/

    uint32_t i = 0, j = 0, k = 0;

    /* These three while loops will interleave    */
    /* the data stored in the two buffers while    */
    /* both buffers still have data, then it will  */
    /* append the remaining data, if any, from the */
    /* buffer that is not empty.                   */
    while(i<N2 && j<N2){
        data.values[k++] = M[i++];
        data.values[k++] = M2[j++];
    }
    while(i<N2)
        data.values[k++] = M[i++];
    while(j<N2)
        data.values[k++] = M2[j++];

    data.length = N;
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
    /* This needs to be rethought
    for(uint32_t i = 0; i < FIR_COEF_COUNT; i++) // Iterates through the size of the x queue.
    {
        y += queue_readElementAt(&xQueue, FIR_COEF_COUNT - i - OFFSET)*firCoefficients[i]; // Convolves x queue with the filter coefficients.
    }
    queue_overwritePush(&yQueue, y); // Add the computed value to the y queue.
    */
    return y; //Return the value as a double.
}

//static uint16_t firDecimationCount = 0;
int decimateData(){
    /* This also needs to be rethought
    if (firDecimationCount == DECIMATION_FACTOR - 1)
    {
        antiAliasFilter(); // Run antialias filter.
        firDecimationCount = 0; // Reset decimation count.
        return true; // Ran the filter.
    }
    firDecimationCount++; // Increment the decimation count each time the filter fuction is called.
    return false; // Didn't run the filter.
    */
    return 1;
}

#if TEST_FUNCTION == 1
void testCode(uint32_t raw_adc_data[], uint32_t N){
#else
void testCode(struct signal data){
#endif
    /* Call whichever function is under test */
    #if TEST_FUNCTION == 1
        struct signal data = reorderData(raw_adc_data, N);
    #elif TEST_FUNCTION == 2
        data = windowData(data);
    #elif TEST_FUNCTION == 3
        data = antiAliasFilter(data);
    #elif TEST_FUNCTION == 4
        data = decimateData(data);
    #endif

    /* Print data to text file to compare with Matlab */
    FILE *dataOut;
    dataOut = fopen("dataOut.txt","wb");
    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++){
        fprintf(dataOut, "%0.3f\n", data.values[i]);
    }

    fclose(dataOut);
}
