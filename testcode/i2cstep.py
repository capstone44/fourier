import smbus

# I2C channel 1 is connected to the GPIO pins
channel = 1

#  MCP4725 defaults to address 0x60
address = 0x4C

# Register addresses (with "normal mode" power-down bits)
reg_write_step = 0x12 # 0b00010010 = This writes to the second output port on the DAC
reg_write_dir = 0x14 # 0b00010100 - This writes to the third output volt
# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)
# 400 half steps / revolution

#voltage = 0xfff #LEFT
#msg = (voltage & 0xff0) >> 4
#msg = [msg, (msg & 0xf) << 4]
#bus.write_i2c_block_data(address, reg_write_dir, msg)

voltage = 0x000 #Right
msg = (voltage & 0xff0) >> 4
msg = [msg, (msg & 0xf) << 4]
bus.write_i2c_block_data(address, reg_write_dir, msg)

# LOW
voltage = 0x000
msg = (voltage & 0xff0) >> 4
msg = [msg, (msg & 0xf) << 4]
bus.write_i2c_block_data(address, reg_write_step, msg)
# Create our 12-bit number representing relative voltage - HIGH
voltage = 0xfff
msg = (voltage & 0xff0) >> 4
msg = [msg, (msg & 0xf) << 4]
bus.write_i2c_block_data(address, reg_write_step, msg)
sleep(0.005)
# LOW
voltage = 0x000
msg = (voltage & 0xff0) >> 4
msg = [msg, (msg & 0xf) << 4]
bus.write_i2c_block_data(address, reg_write_step, msg)
