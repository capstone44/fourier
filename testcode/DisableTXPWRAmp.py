import smbus

# I2C channel 1 is connected to the GPIO pins
channel = 1

#  MCP4725 defaults to address 0x60
address = 0x4C

# Register addresses (with "normal mode" power-down bits)
reg_write_dac = 0x10 #0b00010000 - This writes to the first output port on the DAC

# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)

# Create our 12-bit number representing relative voltage
voltage = 0x000
# Shift everything left by 4 bits and separate bytes
msg = (voltage & 0xff0) >> 4
msg = [msg, (msg & 0xf) << 4]

# Write out I2C command: address, reg_write_dac, msg[0], msg[1]
bus.write_i2c_block_data(address, reg_write_dac, msg)
