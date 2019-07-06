from i2cstep import *
import smbus
from time import sleep
import socket
import sys
import struct
# Import the ADS1x15 module
import Adafruit_ADS1x15


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
    sleep(0.01)
    # HIGH
    voltage = 0xFFF
    msg = (voltage & 0xff0) >> 4
    msg = [msg, (msg & 0xf) << 4]
    bus.write_i2c_block_data(address, reg_write_step, msg)
    # Wait to create a pulse
    sleep(0.01)

# I2C channel 1 is connected to the GPIO pins
channel = 1
address = 0x4C #ADS1115 is configured for address 0x4C

# Averaging factor for power
avgSize = 10

# Register addresses (with "normal mode" power-down bits)
# Initialize I2C (SMBus)
bus = smbus.SMBus(channel)

def convertAngleToPlot(currStep):
    if(currStep < 0):
        angle = (currStep * 0.9) + 360
    else:
        angle = (currStep * 0.9)
    return angle


def runsystem (stepAngle, rotationSize):
    # Setup the socket communication
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    server_address = '/tmp/power_data.sock'
    print >>sys.stderr, 'connecting to %s' % server_address
    try:
        sock.connect(server_address)
    except socket.error, msg:
        print >>sys.stderr, msg
        sys.exit(1)
    # Create an ADS1115 ADC (16-bit) instance.
    adc = Adafruit_ADS1x15.ADS1115()
    GAIN = 1
    adc.start_adc(0,gain=GAIN,data_rate=860)

    vco_reg = 0x16
    voltage = 0x7CF
    bus.write_i2c_block_data(address, vco_reg, [0x7C,0x00])

    step = stepAngle*400/360 # converts angle to required steps for step size
    spin = rotationSize*400/360 # finds total number of steps for desired rotantion

    stepNum = 0

    # Enable Power Amplifier
    pa_reg = 0x10
    bus.write_i2c_block_data(address,pa_reg,[0xFF,0xFF])

    # rotates half way of full rotation size
    set_direction(0)
    for i in range(0,spin/2):
        stepNum = stepNum + 1
        step_once()

    # starts testing the system with steps corresponding to the stepAngle
    set_direction(1)

    # takes a user defined step makes a measurement
    for i in range(0,int(spin/step)):
        # steps the motor to the user defined step
        for j in range(0,int(step)):
            stepNum = stepNum - 1
            step_once()
        # Read Power
        sleep(0.1)
        values = []
        for i in range(avgSize):
            values.append(adc.get_last_result())
            sleep(0.05)

        currPwr = sum(values)/len(values)
        sleep(0.1)
        # Send data to GUI
        currAngle = convertAngleToPlot(stepNum)
        msg = struct.pack("dd",currPwr,currAngle)
        sock.sendall(msg)
        print >>sys.stderr, "Angle: " + str(currAngle) + " Power: " + str(currPwr)
        print >>sys.stderr, 'sent another data point'

    # return back to starting point
    set_direction(0)
    for i in range(0,spin/2):
        stepNum = stepNum + 1
        step_once()
    print >>sys.stderr, 'closing socket'
    sock.close()
    # Disable Power Amplifier
    pa_reg = 0x10
    bus.write_i2c_block_data(address,pa_reg,[0x00,0x00])
