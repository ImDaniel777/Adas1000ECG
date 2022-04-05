import csv
import random
import time

x_value = 0
y_value = 1000
fieldnames = ["LEAD1/LA", "LEAD2/LL", "LEAD3/RA", "V1'/V1", "V2'/V2", "PACE", "RESPPM", "RESPPH", "LOFF", "GPIO", "CRC"]
dataRead = [i for i in range(44)]
sequence = [i for i in range(44)]
bytess = [0x01, 0x02, 0x03, 0x04]
#0x11223344
#onebyte = bytess[0]<<24 | bytess[1]<<16 | bytess[2]<<8 | bytess[3]
onebyte = (''.join(map(str,bytess)))
print(onebyte)
with open('data.csv', 'w') as csvFile:
    csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
    csv_writer.writeheader()
    
while True:
    
    with open('data.csv', 'a') as csvFile:
        csv_writer = csv.DictWriter(csvFile, fieldnames=fieldnames)
            
        info = {
                "LEAD1/LA": dataRead[0],
                "LEAD2/LL":dataRead[1],
                "LEAD3/RA":dataRead[2],
                "V1'/V1":dataRead[3],
                "V2'/V2":dataRead[4],
                "PACE":dataRead[5],
                "RESPPM":dataRead[6],
                "RESPPH":dataRead[7],
                "LOFF":dataRead[8],
                "GPIO":dataRead[9],
                "CRC":dataRead[10]
           }
        csv_writer.writerow(info)
        

        
    time.sleep(1)
        