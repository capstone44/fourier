import time
import numpy as np
import matplotlib.pyplot as plt
# Import the ADS1x15 module.

in_file = open("test.bin","r")
values = np.fromfile(in_file,dtype=np.int32)

plt.figure()
plt.plot(values,"b")
plt.title("Plot of ADC Data")
plt.show()

output_bin = np.array(values,dtype=np.double)
out_file = open("test-radar.bin","w")
output_bin.tofile(out_file)
