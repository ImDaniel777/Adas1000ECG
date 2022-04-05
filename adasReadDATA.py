import spidev
import csv
import RPi.GPIO as GPIO
import time
import binascii
import numpy as np
from itertools import count

t = count()

fieldnames = ["t", "LEAD1/LA", "LEAD2/LL", "LEAD3/RA", "V1'/V1", "V2'/V2", "PACE", "RESPPM", "RESPPH", "LOFF", "GPIO", "CRC"]
outputWord = [0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00,0x00]

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
    writeCmd[1] = regValue & 0xFF0000
    writeCmd[2] = regValue & 0x00FF00
    writeCmd[3] = regValue & 0x0000FF
    CSlow()
    spi.xfer(writeCmd)
    CShigh()
def reset():
    adas1000_setRegister(1, 1)
def adas1000Init():
#    adas1000_setRegister(0x01, 0xF804AE)
#    adas1000_setRegister(0x04, 0x000F88)
#    adas1000_setRegister(0x05, 0xE0000A)
#    adas1000_setRegister(0x0A, 0x078000)

    for i in range (0,16,4):
            CSlow()
            resp = spi.xfer(ADASinit[i:i+4])
            #print('Received: 0x{0}'.format(binascii.hexlify(bytearray(resp))))
            time.sleep(0.005)
            CShigh()

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
def adas1000_readData():
    readData = []
    CSlow()
    spi.xfer([0x40],976000,5,8)
    readData = spi.readbytes(48)
    CShigh()
    #readData = [hex(x) for x in readData]
    readData = readData[::-1]
    readData = [format(x,'02x') for x in readData]
    #print(readData)
    j = 0
    for i in range (0,44,4):
        temp = readData[i:i+4]
        #word = f'0x{0x0A:x}{6:x}{7:x}{8:x}'
        #temp[i] = "0x{:02x}".format(temp[i-1])
        #print('Received: 0x{0}'.format(binascii.hexlify(bytearray(temp))))

        word = (''.join(map(str,temp)))
        outputWord[j] = word
        j=j+1
        print(word)
adas1000Init()
#reset()
adas1000_check()
adas1000_readData()
with open('data.csv', 'w') as csvFile:
    csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
    csv_writer.writeheader()
try:
    while True:
        adas1000_readData()
        with open('data.csv', 'a') as csvFile:
            csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
            
            info = {
                "t":next(t),
                "LEAD1/LA": outputWord[10],
                "LEAD2/LL":outputWord[9],
                "LEAD3/RA":outputWord[8],
                "V1'/V1":outputWord[7],
                "V2'/V2":outputWord[6],
                "PACE":outputWord[5],
                "RESPPM":outputWord[4],
                "RESPPH":outputWord[3],
                "LOFF":outputWord[2],
                "GPIO":outputWord[1],
                "CRC":outputWord[0]
           }
            csv_writer.writerow(info)   
        time.sleep(0.00005)
        
finally:
    spi.close()
    
    
