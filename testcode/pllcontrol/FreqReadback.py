# Simple demo of reading each analog input from the ADS1x15 and printing it to
# the screen.
# Author: Tony DiCola
# License: Public Domain
import time

# Import the ADS1x15 module.
import Adafruit_ADS1x15
import RPi.GPIO as GPIO
import time
import sys

CLK = 11
MOSI = 10
CS = 5

# Create an ADS1115 ADC (16-bit) instance.
adc = Adafruit_ADS1x15.ADS1115()

GAIN = 1

print('Performing Frequency Readback on PLL, via MUXOUT and ADC')
# Print nice channel column headers.
while True:
    value = adc.read_adc(3, gain=GAIN)
    time.sleep(0.5)
