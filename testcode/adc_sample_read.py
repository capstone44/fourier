import time
import numpy as np
import matplotlib.pyplot as plt
# Import the ADS1x15 module.
import Adafruit_ADS1x15

# Create an ADS1115 ADC (16-bit) instance.
adc = Adafruit_ADS1x15.ADS1115()

# Choose a gain of 1 for reading voltages from 0 to 4.09V.
# Or pick a different gain to change the range of voltages that are read:
#  - 2/3 = +/-6.144V
#  -   1 = +/-4.096V
#  -   2 = +/-2.048V
#  -   4 = +/-1.024V
#  -   8 = +/-0.512V
#  -  16 = +/-0.256V
# See table 3 in the ADS1015/ADS1115 datasheet for more info on gain.
GAIN = 16
adc.start_adc(3,gain=GAIN,data_rate=860)
print('Reading ADS1x15 values, press Ctrl-C to quit...')
# Main loop.
values = []
for i in range(10000):
    # Read all the ADC channel values in a list.
        # Read the specified ADC channel using the previously set gain value.
        values.append(adc.get_last_result())

print('Finished sampling')
plt.figure()
plt.plot(values,"b")
plt.title("Plot of ADC Data")
plt.show()

output_bin = np.array(values)
out_file = open("test.bin","w")
output_bin.tofile(out_file)
