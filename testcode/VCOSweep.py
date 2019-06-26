import smbus

# I2C channel 1 is connected to the GPIO pins
channel = 1

#  MCP4725 defaults to address 0x60
address = 0x4C

# Register addresses (with "normal mode" power-down bits)
reg_write_dac = 0x16 #0b00010110 - This writes to the first output port on the DAC

# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)

# Create our 12-bit number representing relative voltage
# Voltage   Frequency
# 0x55F     5.65
# 0x7CF     5.8
# 0xABF     5.95
voltage = 0x7CF

for i in range(0x55F,0xABF):
    voltage = i
    # Shift everything left by 4 bits and separate bytes
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]

    # Write out I2C command: address, reg_write_dac, msg[0], msg[1]
    bus.write_i2c_block_data(address, reg_write_dac, msg)
