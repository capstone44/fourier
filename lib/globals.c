#include "globals.h"

float window_weights[WINDOW_SIZE] = {};

int nextPow2(uint16_t N){
    N--;
    N |= N>>1;
    N |= N>>2;
    N |= N>>4;
    N |= N>>8;
    N++;
    return N;
}
