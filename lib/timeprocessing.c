#include "timeprocessing.h"

const char ADC1_GPIO[] = {9,25,10,22,27,17,18,15,14,24};
const char ADC2_GPIO[] = {20,26,16,19,13,12,7,8,11,21};

void GetV2(uint32_t values[], uint32_t N, float *M){
    uint32_t val;
    for(uint32_t i=0; i<N; i++){
        val = values[i];
        float SignalZero = 0;
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            SignalZero += 1<<((val&(1<<ADC2_GPIO[k]))/(1<<(ADC2_GPIO[k])));
        }
       M[i] = SignalZero;
    }
}

void GetV1(uint32_t values[], uint32_t N, float *M2){
    uint32_t val;
    for(uint32_t i=0; i<N; i++){
        val = values[i];
        float SignalZero = 0;
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            SignalZero += 1<<((val&(1<<ADC1_GPIO[k]))/(1<<(ADC1_GPIO[k])));
        }
        M2[i] = SignalZero;
    }
}

struct signal reorderData(uint32_t raw_adc_data[], uint32_t N){
    signal data;
    /* N can either be the length of the final structure, or the length of raw_adc_data
     * The array initializers and size will either then be N/2, or 2*N, respectively
     */
    float M[N/2];
    float M2[N/2];
    int i = 0, j = 0, k = 0;

    GetV2(raw_adc_data, N/2, M);
    GetV1(raw_adc_data, N/2, M2);

    while(i<sizeof(M) && j<sizeof(M2)){
        data.values[k++] = M[i++];
        data.values[k++] = M2[j++];
    }
    while(i<sizeof(M))
        data.values[k++] = M[i++];
    while(j<sizeof(M2))
        data.values[k++] = M2[j++];

    data.length = N;
    return data;
}

/***********************************************/
/* windowData will accept a a struture with    */
/* an array of ADC values and will multiply    */
/* each value with a corresponding window      */
/* weight found in the globals.c file. The     */
/* struct with the data array will  then be    */
/* returned to the main function.              */
/***********************************************/
struct signal windowData(struct signal data){
    for(uint32_t i=0; i<data.length; i++)
        data.values[i] = data.values[i] * window_weights[i];
    return data;
}

float antiAliasFilter(){
    float y = 0;
    /* This needs to be rethought...TODO
    for(uint32_t i = 0; i < FIR_COEF_COUNT; i++) // Iterates through the size of the x queue.
    {
        y += queue_readElementAt(&xQueue, FIR_COEF_COUNT - i - OFFSET)*firCoefficients[i]; // Convolves x queue with the filter coefficients.
    }
    queue_overwritePush(&yQueue, y); // Add the computed value to the y queue.
    */
    return y; //Return the value as a double.
}

static uint16_t firDecimationCount = 0;
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
/*
void testCode(struct signal data){
    data = reorderData(data);
    FILE *dataOut;

    dataOut = fopen("1mhz_out.txt","wb");

    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++)
        fprintf(dataOut, "%0.3lf\n", data.values[i]);

    fclose(dataOut);
}
*/
