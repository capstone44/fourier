#include "timeprocessing.h"

/***************************************************************/
/* These arrays contain the pin numbers for the different ADCs */
/* which will be used to extract the 10 bit number that        */
/* is the actual value.                                        */
/***************************************************************/
const char ADC1_GPIO[] = {9,25,10,22,27,17,18,15,14,24};
const char ADC2_GPIO[] = {20,26,16,19,13,12,7,8,11,21};

/**********************************************/
/* The output of the ADC does not have the    */
/* data in the correct order, so before and   */
/* processing can be done, the order must     */
/* be corrected. This function calls the      */
/* above functions and then interleaves their */
/* outputs.                                   */
/**********************************************/
struct signal reorderData(uint32_t raw_adc_data[], uint32_t N){
    signal data;    //Create the output structure of the function

    /****************************************************************/
    /* The input value N is the full length of the interleaved ADCs */
    /* we need to create two intermediate arrays that are half of   */
    /* size that will be used to extract and interleave values.     */
    /****************************************************************/
    uint32_t N2 = N/2;
    float M[N2];
    float M2[N2];

    /*****************************************************************/
    /* These are variables used to extract the 10 bit number, val    */
    /* is used to store the current input from the ADC buffer,       */
    /* tmp1; tmp2; and tmp3 are intermediate values, SignalZero1     */
    /* and SignalZero2 are used to accumulate the full 10 bit        */
    /* value. Magnitude is the true bit position in the 10 bit       */
    /* number for the value output at the end of the for loop.       */
    /* Mask1 and mask2 are bit masks that are used to extract the    */
    /* correct bit from val that is needed to form the output value. */
    /*****************************************************************/
    uint32_t val, tmp1, tmp2, tmp3;
    uint32_t magnitude, mask1, mask2;
    float SignalZero1, SignalZero2;

    for(uint32_t i=0; i<N2; i++){
        val = raw_adc_data[i];
        SignalZero1 = SignalZero2 = 0;

        for(uint8_t k=0; k<ADC_LENGTH; k++){
            magnitude = 1 << k;         

            /*********************************************************/
            /* Create the bit mask, perform a bitwise and to extract */
            /* the correct value, shift back to place everything in  */
            /* the correct position, multiply by the magnitude and   */
            /* accumulate to construct the 10 bit integer.           */
            /*********************************************************/
            mask1 = 1 << ADC2_GPIO[k];  
            tmp1 = val & mask1;    
            tmp2 = tmp1 >> ADC2_GPIO[k];         
            tmp3 = magnitude * tmp2;    
            SignalZero1 += tmp3;
            
            mask2 = 1 << ADC1_GPIO[k];
            tmp1 = val & mask2;
            tmp2 = tmp1 >> ADC1_GPIO[k];
            tmp3 = magnitude * tmp2;
            SignalZero2 += tmp3;
        }
        /********************************************/
        /* Store each 10 bit integer in the correct */
        /* position of the intermediate arrays.     */
        /********************************************/
        M[i] = SignalZero1;
        M2[i] = SignalZero2;
    }

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
    data.fs = 6173300;
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
struct signal windowData(struct signal data){;
    for(uint32_t i=0; i<data.length; i++)
        data.values[i] *= window_weights[i];
    return data;
}

/**********************************************/
/* decimateData will change the length to the */
/* new decimated length and also update the   */
/* sampling frequency. It will then go        */
/* through the data values and keep every     */
/* third value, discarding the rest.          */
/**********************************************/
struct signal decimateData(struct signal data){
    data.length = data.length/DECIMATION_FACTOR;
    data.fs = data.fs/DECIMATION_FACTOR;
    uint32_t j = 0;
    for (uint32_t i=0; i<data.length; i++){
        data.values[i] = data.values[j];
        j += DECIMATION_FACTOR;
    }
    return data;
}

struct signal zeroPad(struct signal data){
    uint16_t N = data.length;
    data.length = FFT_SIZE;
    for(uint32_t i = N; i<data.length; i++)
        data.values[i] = 0;
    return data;
}

/**************************************************/
/* This test code is controlled by the #define in */
/* timeprocessing.h, it will run whichever        */
/* function is under test and print the values    */
/* to an output file for comparison with Matlab.  */
/**************************************************/
#if TEST_FUNCTION_TIME == 1
void testCodeTime(uint32_t raw_adc_data[], uint32_t N){
#else
void testCodeTime(struct signal data){
#endif
    /* Call whichever function is under test */
    #if TEST_FUNCTION_TIME == 1
        struct signal data = reorderData(raw_adc_data, N);
    #elif TEST_FUNCTION_TIME == 2
        data = decimateData(data);
    #elif TEST_FUNCTION_TIME == 3
        data = windowData(data);
    #endif

    /* Print data to text file to compare with Matlab */
    FILE *dataOut;
    dataOut = fopen("dataOut.txt","wb");
    if(dataOut == NULL)
        printf("Cannot create file\n\r");

    for(uint32_t i=0; i<data.length; i++){
        fprintf(dataOut, "%f\n", data.values[i]);
    }

    fclose(dataOut);
}
