import numpy as np
import matplotlib.pyplot as plt
from array import array
import scipy.io
import math
RawData ="beat_test.bin"
f = open(RawData, "r")
a = np.fromfile(f, dtype=np.uint32)
#t = 2668891
V = a[:-1].copy()
#T = [ (( x * t ) / ( 1000.0*len(V) )) for x in 2*range(len(V))]

ADC1_GPIO = [9,25,10,22,27,17,18,15,14,24]

ADC2_GPIO = [20,26,16,19,13,12,7,8,11,21]

print(type(20))

ADC1len = len(ADC1_GPIO)
ADC2len = len(ADC2_GPIO)

def GetV2(Volts):
    Signal = []
    Map = np.zeros((len(V),ADC2len), dtype=np.int)
    for i in range(len(Volts)):
        val = int(V[i])
        #print("Value of V[i]: ", V[i], "Value of val: ", val)
        SignalZero = 0
        for k in range(ADC2len):
            Map[i][k] = (val & 2**k)/2**k
        for k in range(ADC2len):
            SignalZero += 2**k*((val & 2**ADC2_GPIO[k])/2**ADC2_GPIO[k])
        Signal.append(SignalZero)

    return Signal,Map

def GetV1(Volts):
    Signal = []
    Map = np.zeros((len(V),ADC1len), dtype=np.int)
    for i in range(len(Volts)):
        val = int(V[i])
        SignalZero = 0
        for k in range(ADC1len):
            Map[i][k] = (val & 2**k)/2**k
        for k in range(ADC1len):
            SignalZero += 2**k*((val & 2**ADC1_GPIO[k])/2**ADC1_GPIO[k])
        Signal.append(SignalZero)

    return Signal,Map

M = GetV2(V)[0]
M2 = GetV1(V)[0]

SigOut = [];

for z in range(len(M)):
    SigOut.append(M[z]);
    SigOut.append(M2[z]);
    z += 1;

#scipy.io.savemat('750k.mat', mdict={'data': SigOut})

#outfile = open('750k-processed.bin','wb')

#array('h',SigOut).tofile(outfile)

plt.figure(figsize=(15,5))
plt.plot(SigOut,"b")
plt.title("Plots "+RawData+' .')
plt.show()
