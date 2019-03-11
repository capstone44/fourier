#include "frequencyprocessing.h"

struct signal keepPositiveFreq(struct signal data){
    data.length = data.length/2;
    data.delta_f = data.fs/data.length;
    uint32_t i = 0;
    while(i<data.fs/2){
        data.frequencies[i] = i;
        i = i*data.delta_f;
    }
    return data;
}

struct signal calculateMagSquared(struct signal real_data, struct signal imag_data){
    struct signal psdx;
    psdx.length = real_data.length;
    psdx.fs = real_data.fs;
    float scaler = 1/(psdx.fs*psdx.length);
    for(uint32_t i=0; i<psdx.length; i++){
        psdx.frequencies[i] = real_data.frequencies[i];
        real_data.values[i] *= real_data.values[i];
        imag_data.values[i] *= imag_data.values[i];
        if(i != 0 && i != psdx.length){
            psdx.values[i] = 2*scaler*(real_data.values[i]+imag_data.values[i]);
        }
        else
        {
            psdx.values[i] = scaler*(real_data.values[i]+imag_data.values[i]);
        }
    }
    return psdx;
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
    float right value = val.right_value;
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
