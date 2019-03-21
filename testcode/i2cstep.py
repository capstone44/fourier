import smbus

def step_once(direction):
    if direction == 1:
        dir_voltage = 0x000
    else:
        dir_voltage = 0xFFF

    msg = (dir_voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_dir, msg)

    # LOW
    voltage = 0x000
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)
    # HIGH
    voltage = 0xFFF
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)
    # Wait to create a pulse
    sleep(0.005)
    # LOW
    voltage = 0x000
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)

# I2C channel 1 is connected to the GPIO pins
channel = 1
address = 0x4C #ADS1115 is configured for address 0x4C

# Register addresses (with "normal mode" power-down bits)
reg_write_step = 0x12 # 0b00010010 = This writes to the second output port on the DAC
reg_write_dir = 0x14 # 0b00010100 - This writes to the third output volt
# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)
# 400 half steps / revolution
for i in range(0,400):
    step_once(1)
