#include <stdio.h>
#include <stdlib.h>
#include "timeprocessing.h"
#include "globals.h"

void GetV2(float values[], uint32_t N, *M){
    float Signal[N];
    int32_t val;
    for(uint32_t i=0; i<N; i++){
        val = int(values[i]);
        float SignalZero = 0;
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            SignalZero += 1<<((val&(1<<ADC2_GPIO[k]))/(1<<(ADC2_GPIO[k])));
        }
        M[i] = SignalZero;
    }
}

void GetV1(float values[], uint32_t N, *M2){
    float Signal[N];
    int32_t val;
    for(uint32_t i=0; i<N; i++){
        val = int(values[i]);
        float SignalZero = 0;
        for(uint8_t k=0; k<ADC_LENGTH; k++){
            SignalZero += 1<<((val&(1<<ADC1_GPIO[k]))/(1<<(ADC1_GPIO[k])));
        }
        M2[i] = SignalZero;
    }
}

struct signal reorderData(struct signal data){
    data.length--;
    float M[data.length];
    float M2[data.length];
    int i = 0, j = 0, k = 0;

    GetV2(data.values, data.length, M);
    GetV1(data.values, data.length, M2);

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
    data = reorderData(data);
    FILE *dataOut;

    dataOut = fopen("1mhz_out.txt","wb");

    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++)
        fprintf(dataOut, "%0.3lf\n", data.values[i]);

    fclose(dataOut);
}