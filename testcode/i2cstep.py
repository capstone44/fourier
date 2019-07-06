import smbus
from time import sleep

def set_direction(direction):
    reg_write_dir = 0x14 # 0b00010100 - This writes to the third output volt
    if direction == 0:
        dir_voltage = 0x000
    else:
        dir_voltage = 0xFFF

    msg = (dir_voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_dir, msg)

def step_once():
    reg_write_step = 0x12 # 0b00010010 = This writes to the second output port on the DAC

    # LOW
    voltage = 0x000
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)
    sleep(0.1)
    # HIGH
    voltage = 0xFFF
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)
    # Wait to create a pulse
    sleep(0.1)
