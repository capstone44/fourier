#include "timeprocessing.h"

const char ADC1_GPIO[] = {9,25,10,22,27,17,18,15,14,24};
const char ADC2_GPIO[] = {20,26,16,19,13,12,7,8,11,21};

float* GetV2(uint32_t values[], uint32_t N){
    uint32_t val,tmp;
    float SignalZero;
    uint32_t shift;

    static float M[N];

    printf("Value of N in GetV2: %d\n\r Value of sizeof(M) %ld\n\r", N, sizeof(M));

    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in an unsigned int.    */
    for(uint32_t i=0; i<N; i++){
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

float* GetV1(uint32_t values[], uint32_t N){
    int32_t val,tmp;
    float SignalZero;
    uint32_t shift;
    
    static float M2[N];

    /* Iterate over the length of the data. */
    /* Reset all variables to zero and grab */
    /* current value in an unsigned int.    */
    for(uint32_t i=0; i<N; i++){
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
    printf("Value of N2: %d\n\r", N2);
    float *M;
    float *M2;
    int i = 0, j = 0, k = 0;

    M = GetV2(raw_adc_data, N2);
    M2 = GetV1(raw_adc_data, N2);

    /* These three while loops will interleave    */
    /* the data stored in the two buffers while    */
    /* both buffers still have data, then it will  */
    /* append the remaining data, if any, from the */
    /* buffer that is not empty.                   */
    while(i<sizeof(M) && j<sizeof(M2)){
        data.values[k++] = M[i++];
        data.values[k++] = M2[j++];
    }
    printf("Size of M: %ld\n\r Value of i: %d \n\r", sizeof(M), i);
    while(i<sizeof(M)){
        data.values[k++] = M[i++];
        printf("In excess while loop\n\r");
    }
    printf("Size of M2: %ld\n\r Value of j: %d \n\r", sizeof(M2), j);
    while(j<sizeof(M2))
        data.values[k++] = M2[j++];

    printf("About to set data.length\n\r");
    data.length = N;
    printf("Returning from reorderData\n\r");
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

    printf("Back in test function\n\r");

    /* Print data to text file to compare with Matlab */
    FILE *dataOut;
    dataOut = fopen("1mhz_out.txt","wb");
    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++){
        printf("Value of i: %d\n\r Value of data.length: %d", i, data.length);
        fprintf(dataOut, "%0.3f\n", data.values[i]);
    }

    fclose(dataOut);
}
