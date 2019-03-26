#include "frequencyprocessing.h"

/*******************************************************/
/* To calculate the magnitude squared, square the real */
/* and imaginary values and then add them and then     */
/* scale the output. Since we only have the positive   */
/* frequencies we need to double all all of the values */
/* to account for (an assumed) symmetric negative      */
/* frequency composition. The first and last values    */
/* do not have a negative frequency equivalent and are */
/* thus left alone. The new frequency array will be    */
/* same as both of the real and imaginary frequency    */
/* arrays.                                             */
/*******************************************************/
struct signal calculateMagSquared(struct signal real_data, struct signal imag_data){
    struct signal psdx;
    double delta_f = (double) real_data.fs / (double) ((real_data.length - 1) * 2);
    uint32_t j = 0;
    for(uint32_t i=0; i<real_data.length; i++){
        j = i*delta_f;
        psdx.frequencies[i] = j;
    }
    psdx.length = real_data.length;
    psdx.fs = real_data.fs;
    uint64_t scaler = (uint64_t) psdx.fs * (uint64_t) psdx.length * 2;
    for(uint32_t i=0; i<psdx.length+1; i++){
        real_data.values[i] *= real_data.values[i];
        imag_data.values[i] *= imag_data.values[i];
        if(i != 0 && i != psdx.length){
            psdx.values[i] = 2*(real_data.values[i]+imag_data.values[i])/scaler;
        }
        else
        {
            psdx.values[i] = (real_data.values[i]+imag_data.values[i])/scaler;
        }
    }
    return psdx;
}

/*****************************************************/
/* The output of the magnitude squared function will */
/* have several large peaks, one that is especially  */
/* problematic happens at DC. In order to attenuate  */
/* the large DC peak and a few others multiply each  */
/* frequency bin by a weight created in Matlab and   */
/* stored in a variable initialized in               */
/* frequencyprocessing.h.                            */
/*****************************************************/
struct signal filter(struct signal data){
    for(uint32_t i=0; i<data.length; i++){
        data.values[i] *= filter_mag_squared[i];
    }
    return data;
}

/*******************************************************/
/* The interpolate function needs the peak value of    */
/* the DFT with its index and the values directly      */
/* to the left and right with their indices. This      */
/* function will iterate through the psdx data and     */
/* keeps track of the current max value and its        */
/* index, if it comes across a larger value, they      */
/* are updated. After iterating through the data,      */
/* the left and right indices are calculated from      */
/* the max index, which are then used to calculate     */
/* their data values. The true max value and           */
/* frequency are then calculated using a parabolic     */
/* curve fit. All values are stored for interpolation. */
/*******************************************************/
struct max_values findPeak(struct signal psdx){
    struct max_values val;
    double max_value = 0;
    double left_value;
    double right_value;
    double actual_max_value;
    double actual_max_frequency;
    uint32_t max_frequency;
    uint32_t left_index;
    uint32_t right_index;
    double tmp;

    for(uint32_t i=0; i<psdx.length; i++){
        tmp = psdx.values[i];
        if(tmp > max_value){
            max_value = tmp;
            max_frequency = i;
        }
    }

    left_index = max_frequency - 1;
    right_index = max_frequency + 1;
    left_value = psdx.values[left_index];
    right_value = psdx.values[right_index];

    /* To better understand this calculation visit the following website:               */
    /* https://ccrma.stanford.edu/~jos/sasp/Quadratic_Interpolation_Spectral_Peaks.html */
    actual_max_frequency = (left_value - right_value)/(2*(left_value + right_value - 2*max_value));
    actual_max_value = max_value - (left_value - right_value)*actual_max_frequency/4;
    actual_max_frequency += psdx.frequencies[max_frequency];

    val.actual_max_frequency = actual_max_frequency;
    val.actual_max_value = actual_max_value;
    val.right_value = right_value;
    val.left_value = left_value;
    val.left_index = left_index;
    val.right_index = right_index;
    return val;
}

/****************************************************************************/
/* First store all values from the input val into local variables for space */
/* management. The function will then iterate over the length of psdx and   */
/* calculate the interpolated value using lagrangian interpolation. If the  */
/* interpolated value is negative, set it to 0.                             */
/****************************************************************************/
void interpolate(struct signal psdx, struct max_values val, double *buf){
    double tmp, P1, P2, P3, output;
    double left_freq = psdx.frequencies[val.left_index];
    double right_freq = psdx.frequencies[val.right_index];
    double max_freq = val.actual_max_frequency;
    double max_value = val.actual_max_value;
    double left_value = val.left_value;
    double right_value = val.right_value;

    uint16_t startAvg = 200;
    uint16_t endAvg = 2000;
    uint16_t avgLength = endAvg - startAvg;
    double noiseAvg = 0;
    double threshold = 0;
    for(int32_t i=startAvg; i<endAvg; i++){
        noiseAvg += psdx.frequencies[i];
    }
    noiseAvg /= avgLength;
    threshold = 20*noiseAvg;

    for(uint32_t i=0; i<psdx.length-1; i++){
        tmp = psdx.frequencies[i];

        /* To better understand this calculation visit the following website: */
        /* https://www.math.usm.edu/lambers/mat772/fall10/lecture5.pdf        */
        P1 = ((tmp-max_freq)*(tmp-right_freq))*left_value/((left_freq-max_freq)*(left_freq-right_freq));
        P2 = ((tmp-left_freq)*(tmp-right_freq))*max_value/((max_freq-left_freq)*(max_freq-right_freq));
        P3 = ((tmp-left_freq)*(tmp-max_freq))*right_value/((right_freq-left_freq)*(right_freq-max_freq));
        output = P1 + P2 + P3;
        
        if(output < 0.0){
            if(psdx.frequencies[i] > threshold){
                buf[i] = noiseAvg;
            }
            else{
                buf[i] = psdx.frequencies[i];
            }
        }
        else{
            buf[i] = output;
        }
    }

}

/*************************************************************/
/* Calculate power using the DFT form of Parseval's Theorem. */
/*************************************************************/
double calculatePower(double *buf, uint32_t N){
    double P = 0;
    for(uint32_t i=0; i<N; i++){
        P += buf[i];
    }
    P /= N;
    return P;
}

/**************************************************/
/* This test code is controlled by the #define in */
/* timeprocessing.h, it will run whichever        */
/* function is under test and print the values    */
/* to an output file for comparison with Matlab.  */
/**************************************************/
#if TEST_FUNCTION_FREQ == 1 || TEST_FUNCTION_FREQ == 2
void testCodeFreq(struct signal real_data, struct signal imag_data){
#elif TEST_FUNCTION_FREQ == 5
void testCodeFreq(struct signal data, struct max_values val){
#elif TEST_FUNCTION_FREQ == 6
void testCodeFreq(struct signal data, double *buf){
#else
void testCodeFreq(struct signal data){
#endif
    /* Call whichever function is under test */
    #if TEST_FUNCTION_FREQ == 1
        real_data = keepPositiveFreq(real_data);
        imag_data = keepPositiveFreq(imag_data);
    #elif TEST_FUNCTION_FREQ == 2
        struct signal psdx = calculateMagSquared(real_data, imag_data); 
    #elif TEST_FUNCTION_FREQ == 3
        data = filter(data);
    #elif TEST_FUNCTION_FREQ == 4
        struct max_values val = findPeak(data);
    #elif TEST_FUNCTION_FREQ == 5
        float buf[data.length];
        interpolate(data, val, buf);
    #elif TEST_FUNCTION_FREQ == 6
        double Power = calculatePower(buf, data.length);
    #endif

    /* Print data to text file to compare with Matlab */
    #if TEST_FUNCTION_FREQ == 1
    FILE *realDataOut;
    FILE *imagDataOut;
    realDataOut = fopen("realDataOut.txt","wb");
    imagDataOut = fopen("imagDataOut.txt","wb");
    if(realDataOut == NULL)
        printf("Cannot create file\n\r");
    if(imagDataOut == NULL)
        printf("Cannot create file\n\r");
    #else
    FILE *dataOut;
    dataOut = fopen("dataOut.txt","wb");
    if(dataOut == NULL)
        printf("Cannot create file\n\r");
    #endif

    #if TEST_FUNCTION_FREQ == 6
        printf("Power value is : %f\n\r", Power);

    #elif TEST_FUNCTION_FREQ == 4
        printf("Interpolated max value : %f\n\r", val.actual_max_value);
        printf("Interpolated max value frequency : %f\n\r", val.actual_max_frequency);
        printf("Interpolated left value and index: %f %d\n\r", val.left_value, val.left_index);
        printf("Interpolated right value and index: %f %d\n\r", val.right_value, val.right_index);
    #else
    #if TEST_FUNCTION_FREQ == 1 || TEST_FUNCTION_FREQ == 2
    for(uint32_t i=0; i<real_data.length; i++){  
    #else
    for(uint32_t i=0; i<data.length; i++){
    #endif
        #if TEST_FUNCTION_FREQ == 1
            fprintf(realDataOut, "%f\n", real_data.values[i]);
            fprintf(imagDataOut, "%f\n", imag_data.values[i]);
        #elif TEST_FUNCTION_FREQ == 5
            fprintf(dataOut, "%f\n", buf[i]);
        #elif TEST_FUNCTION_FREQ == 2
            fprintf(dataOut, "%f\n", psdx.values[i]);
        #else
            fprintf(dataOut, "%g\n", data.values[i]);
        #endif
    }
    #endif

    #if TEST_FUNCTION_FREQ == 1
        fclose(realDataOut);
        fclose(imagDataOut);
    #else
        fclose(dataOut);
    #endif
}
