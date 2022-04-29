import spidev
import csv
import RPi.GPIO as GPIO
import time
import binascii
import numpy as np
from itertools import count

drdy = 3

t = count()

fieldnames = ["t", "LEAD1/LA", "LEAD2/LL", "LEAD3/RA", "V1'/V1", "V2'/V2", "PACE", "RESPPM", "RESPPH", "LOFF", "GPIO", "CRC"]
outputWord = [ 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00]

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 976000  #adas1000 frame rate = 2kHz ==> min spi clock = 768khz
spi.bits_per_word = 8

ADASinit = [0x81, 0xF8, 0x04, 0xAE,
            0x84, 0x00, 0x0F, 0x88,
            0x85, 0xE0, 0x00, 0x0A,
            0x8A, 0X07, 0X80, 0X00]

GPIO.setmode(GPIO.BOARD)
GPIO.setup(3,GPIO.OUT) #pin used as CS
def CSlow():
    GPIO.output(3, GPIO.LOW)
def CShigh():
    GPIO.output(3, GPIO.HIGH)
def adas1000_setRegister(regAddress, regValue):
    writeCmd = [0x00, 0x00, 0x00, 0x00]
    writeCmd[0] = 0x80 + regAddress
    writeCmd[1] = (regValue & 0xFF0000) >> 16;
    writeCmd[2] = (regValue & 0x00FF00) >> 8;
    writeCmd[3] = (regValue & 0x0000FF)
    CSlow()
    spi.writebytes(writeCmd)
    CShigh()
def reset():
    adas1000_setRegister(0x01, 1)
    adas1000_setRegister(0x00, 0)
def adas1000_wait():
    readCmd = [0x00, 0x00, 0x00, 0x00]
    readData = [0x00, 0x00, 0x00, 0x00]
    readCmd[0] = 0x01;
    CSlow()
    spi.writebytes(readCmd)
    CShigh()
    CSlow()
    readData = spi.readbytes(4)
    CShigh()
    readData = [hex(x) for x in readData]
    adas1000_setRegister(0x00, 0) 
def adas1000Init():
    adas1000_setRegister(0x01, 0xF804AE)
    adas1000_setRegister(0x04, 0x000F88)
    adas1000_setRegister(0x05, 0xE0000A)
    adas1000_setRegister(0x0A, 0x079400)

def adas1000_readRegister(reg_addr):
    readCmd = [0x00, 0x00, 0x00, 0x00]
    readData = [0x00, 0x00, 0x00, 0x00]
    readCmd[0] = reg_addr
    CSlow()
    spi.writebytes(readCmd)
    CShigh()
    CSlow()
    readData = spi.readbytes(4)
    CShigh()
    readData = [hex(x) for x in readData]
    adas1000_setRegister(0x00, 0)    
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
#     print("HEADER\n")
#     adas1000_readRegister(0x40)
def adas1000_readData():
    readData = []
    CSlow()
    #spi.xfer([0x40],976000,5,8)
    spi.writebytes([0x40])
    readData = spi.readbytes(40)
    CShigh()
    #adas1000_setRegister(0x00, 0) 
    readData = readData[::-1]
    readData = [format(x,'02x') for x in readData]
   # print(readData)
    j = 0
    for i in range (0,40,4):
        temp = readData[i:i+4]
        word = (''.join(map(str,temp)))
        outputWord[j] = word
        j=j+1
        print(word)
    print("\n")
def adas1000_readData2(n):
    while n != 0:
        adas1000_setRegister(0x00, 0) 
        readData = []
        adas1000Init()
        CSlow()
        

        #spi.xfer([0x40],976000,5,8)
        spi.writebytes([0x40])
        readData = spi.readbytes(36)
        CShigh()
        readData = readData[::-1]
        readData = [format(x,'02x') for x in readData]
        print(readData)
        j = 0
        for i in range (0,36,4):
            temp = readData[i:i+4]
            word = (''.join(map(str,temp)))
            outputWord[j] = word
            j=j+1
            #print(word)
        print("\n")
        n = n-1
#Example how to read and write a register
#adas1000_setRegister(0x05, 0xE0000B)
#adas1000_readRegister(0x05)
#adas1000_setRegister(0x00, 0)    
reset()
adas1000Init()
#time.sleep(0.100)
adas1000_check()
adas1000_readData()
#time.sleep(0.100)
adas1000_readData()
#time.sleep(0.010)    
adas1000_readData()

#reset()        
#adas1000Init()
#adas1000_check()
#adas1000_readData()
#with open('data.csv', 'w') as csvFile:
#    csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
#    csv_writer.writeheader()
#try:
#    while True:
#        adas1000Init()
#        #if GPIO.input(drdy) == 0 :
#        adas1000_readData()
#        with open('data.csv', 'a') as csvFile:
#            csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
#            
#            info = {
#                "t":next(t),
#                "LEAD1/LA": outputWord[7],
#                "LEAD2/LL":outputWord[6],
#                "LEAD3/RA":outputWord[5],
#                "V1'/V1":outputWord[4],
#                "V2'/V2":outputWord[3],
#                "PACE":outputWord[2],
#                "RESPPM":outputWord[1],
#                #"RESPPH":outputWord[3],
#                "LOFF":outputWord[0],
#                #"GPIO":outputWord[1],
#                #"CRC":outputWord[0]
#           }
#            csv_writer.writerow(info)   
#        time.sleep(0.010)
#
#finally:
#    spi.close()
    
    
