import spidev
import csv
import RPi.GPIO as GPIO
import time
import binascii
import numpy as np

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 976000  #adas1000 frame rate = 2kHz ==> min spi clock = 768khz
spi.bits_per_word = 8

ADASinit = [0x81, 0xF8, 0x04, 0xAE,0x84, 0x00, 0x0F, 0x88,0x85, 0xE0, 0x00, 0x0A,0x8A, 0X07, 0X90, 0X00]

GPIO.setmode(GPIO.BOARD)
GPIO.setup(3,GPIO.OUT) #pin used as CS
def CSlow():
    GPIO.output(3, GPIO.LOW)
def CShigh():
    GPIO.output(3, GPIO.HIGH)
    
def adas1000Init():
    for i in range (0,16,4):
            GPIO.output(3, GPIO.LOW)
            resp = spi.xfer(ADASinit[i:i+4])
            #print('Received: 0x{0}'.format(binascii.hexlify(bytearray(resp))))
            time.sleep(0.005)
            GPIO.output(3,GPIO.HIGH)
def adas1000_readRegister(reg_addr):
    readCmd = [0x00, 0x00, 0x00, 0x00]
    readData = [0x00, 0x00, 0x00, 0x00]
    readCmd[0] = reg_addr
    CSlow()
    spi.xfer(readCmd)
    CShigh()
    CSlow()
    readData = spi.xfer(readData)
    CShigh()
    readData = [hex(x) for x in readData]
    print(readData)
def adas1000_check():
     print("ECGCTL\n")
     adas1000_readRegister(0x01)
     print("PACECTL\n")
     adas1000_readRegister(0x04)
     print("CMREFCTL\n")
     adas1000_readRegister(0x05)
     print("FRCMTL\n")
     adas1000_readRegister(0x0A)
adas1000Init()
adas1000_check()
try:
    while True:
        time.sleep(1)
finally:
    spi.close()
