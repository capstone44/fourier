#!/usr/bin/env python
from adf4158 import ADF4158
import RPi.GPIO as GPIO
import time
import sys

pll = ADF4158()

f0 = 5.7998e9
bw = 150e6
tsweep = 120.0 #seconds for the whole sweep
fpd_freq = 10e6 #The PLL's reference is at 10 MHz
step_size = int(10e6) # Step Size of 1 MHz

status = pll.freq_to_regs(f0, fpd_freq, bw, tsweep)
CLK = 11
MOSI = 10
CS = 5

def setupSpiPins(clkPin, mosiPin, csPin):
    ''' Set all pins as an output except MISO (Master Input, Slave Output)'''
    GPIO.setup(clkPin, GPIO.OUT)
    GPIO.setup(mosiPin, GPIO.OUT)
    GPIO.setup(csPin, GPIO.OUT)


def sendCommand(data, register, clkPin, mosiPin, csPin):
    if (register > 7 or register < 0):
        print "Invalid Register!"
        return

    # Start the read with both clock and chip select low
    GPIO.output(csPin, GPIO.LOW)
    GPIO.output(clkPin, GPIO.HIGH)
    # Send these at the end...
    control_bits = register
    # Break data into 4 bytes to transmit one at a time...
    byte_low = (data & 0xFF) | control_bits
    byte_lowmid = ((data >> 8) & 0xFF)
    byte_highmid =  ((data >> 16) & 0xFF)
    byte_high = (data >> 24)

    sendBits(byte_high, 8, clkPin, mosiPin)
    sendBits(byte_highmid, 8, clkPin, mosiPin)
    sendBits(byte_lowmid, 8, clkPin, mosiPin)
    sendBits(byte_low, 8, clkPin, mosiPin)

    # Set chip select high to end the read
    GPIO.output(csPin, GPIO.HIGH)

def sendBits(data, numBits, clkPin, mosiPin):
    ''' Sends 1 Byte or less of data'''

    data <<= (8 - numBits)

    for bit in range(numBits):
        # Set RPi's output bit high or low depending on highest bit of data field
        if data & 0x80:
            GPIO.output(mosiPin, GPIO.HIGH)
        else:
            GPIO.output(mosiPin, GPIO.LOW)

        # Advance data to the next bit
        data <<= 1

        # Pulse the clock pin HIGH then immediately low
        GPIO.output(clkPin, GPIO.HIGH)
        GPIO.output(clkPin, GPIO.LOW)

if __name__ == '__main__':
        GPIO.setmode(GPIO.BCM)
        setupSpiPins(CLK, MOSI, CS)
        print "Now loading base 5.8 GHz tone registers"
        # Send the commands
        sendCommand(pll.registers[7],7,CLK, MOSI, CS)
        pll.write_value(step_sel=0)
        sendCommand(pll.registers[6],6,CLK, MOSI, CS)
        pll.write_value(step_sel=1)
        sendCommand(pll.registers[6],6,CLK, MOSI, CS)
        pll.write_value(dev_sel=0)
        sendCommand(pll.registers[5],5,CLK, MOSI, CS)
        pll.write_value(dev_sel=1)
        sendCommand(pll.registers[5],5,CLK, MOSI, CS)
        sendCommand(pll.registers[4],4,CLK, MOSI, CS)
        sendCommand(pll.registers[3],3,CLK, MOSI, CS)
        sendCommand(pll.registers[2],2,CLK, MOSI, CS)
        sendCommand(pll.registers[1],1,CLK, MOSI, CS)
        sendCommand(pll.registers[0],0,CLK, MOSI, CS)
        sendCommand(pll.registers[0],0,CLK, MOSI, CS)
        print "Now beginning sweep with bandwidth: 150 MHz around 5.8 GHz"
        fstart = f0 - (bw/2)
        fstop = f0 + (bw/2)
        steps = range(int(fstart), int(fstop), step_size)
        step_delay = tsweep / len(steps)
        print "Start Frequency: " + str(fstart)
        print "Stop Frequency: " + str(fstop)
        print "Executing " + str(len(steps)) + " steps with " + str(step_delay) + " seconds between them"
        for i in steps:
            n = int((i/(fpd_freq)))
            frac_msb = int( ((i/(fpd_freq)) - n)* (1 << 12) )
            frac_lsb = int( (((i/(fpd_freq)) - n)*(1 << 12) - frac_msb)*(1 << 13) )
            pll.write_value(n=n)
            pll.write_value(frac_msb=frac_msb)
            pll.write_value(frac_lsb=frac_lsb)
            sendCommand(pll.registers[1],1,CLK, MOSI, CS)
            sendCommand(pll.registers[0],0,CLK, MOSI, CS)
            sendCommand(pll.registers[0],0,CLK, MOSI, CS)
            time.sleep(step_delay)

        print "Finished sweep, cleaning up"
        GPIO.cleanup()
        sys.exit(0)
