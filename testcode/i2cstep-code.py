from i2cstep import *

# I2C channel 1 is connected to the GPIO pins
channel = 1
address = 0x4C #ADS1115 is configured for address 0x4C

# Register addresses (with "normal mode" power-down bits)
# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)
set_direction(0)
# 400 half steps / revolution
for i in range(0,400):
    step_once()

