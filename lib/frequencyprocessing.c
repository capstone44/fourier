#include "frequencyprocessing.h"

struct signal keepPositiveFreq(struct signal data){
    data.delta_f = data.fs/data.length;
    data.length = data.length/2;
    uint32_t j = 0;
    for(uint32_t i=0; i<data.length; i++){
        j = i*data.delta_f;
        data.frequencies[i] = j;
    }
    return data;
}

struct signal calculateMagSquared(struct signal real_data, struct signal imag_data){
    struct signal psdx;
    psdx.length = real_data.length;
    psdx.fs = real_data.fs;
    uint64_t scaler = (uint64_t) psdx.fs * (uint64_t) psdx.length * 2;
    for(uint32_t i=0; i<psdx.length+1; i++){
        psdx.frequencies[i] = real_data.frequencies[i];
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

struct signal filter(struct signal data){
    for(uint32_t i=0; i<data.length; i++){
        data.values[i] *= filter_mag_squared[i];
    }
    return data;
}

struct max_values findPeak(struct signal psdx){
    struct max_values val;
    float max_value = 0;
    float left_value;
    float right_value;
    float actual_max_value;
    float actual_max_frequency;
    uint32_t max_frequency;
    uint32_t left_index;
    uint32_t right_index;
    float tmp;

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
    actual_max_frequency = (left_value - right_value)/(2*(left_value + right_value - 2*max_value));
    actual_max_value = max_value - (left_value - right_value)*actual_max_frequency/4;
    actual_max_frequency += max_frequency;
    val.actual_max_frequency = actual_max_frequency;
    val.actual_max_value = actual_max_value;
    val.right_value = right_value;
    val.left_value = left_value;
    val.left_index = left_index;
    val.right_index = right_index;
    return val;
}

void interpolate(struct signal psdx, struct max_values val, float *buf){
    float tmp, P1, P2, P3;
    float left_freq = psdx.frequencies[val.left_index];
    float right_freq = psdx.frequencies[val.right_index];
    float max_freq = val.actual_max_frequency;
    float max_value = val.actual_max_value;
    float left_value = val.left_value;
    float right_value = val.right_value;
    for(uint32_t i=0; i<psdx.length-1; i++){
        tmp = psdx.frequencies[i];
        P1 = ((tmp-max_freq)*(tmp-right_freq))*left_value/((left_freq-max_freq)*(left_freq-right_freq));
        P2 = ((tmp-left_freq)*(tmp-right_freq))*max_value/((max_freq-left_freq)*(max_freq-right_freq));
        P3 = ((tmp-left_freq)*(tmp-max_freq))*right_value/((right_freq-left_freq)*(right_freq-max_freq));
        buf[i] = P1 + P2 + P3;
        //if(!P(i))
        //    P(i) = 0;
    }

}

float calculatePower(float *buf, uint32_t N, float delta_f){
    float P = 0;
    float scaler = delta_f/2;
    for(uint32_t i=1; i<=N; i++){
        P += scaler * (buf[i] + buf[i-1]);
    }
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
#elif TEST_FUNCTION_FREQ == 6
void testCodeFreq(struct signal data, *buf){
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
        struct max_values val = findPeak(psdx);
    #elif TEST_FUNCTION_FREQ == 5
        float buf[psdx.length];
        interpolate(psdx, val, buf);
    #elif TEST_FUNCTION_FREQ == 6
        float Power = calculatePower(buf, psdx.length, psdx.delta_f);
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
            fprintf(dataOut, "%f\n", data.values[i]);
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