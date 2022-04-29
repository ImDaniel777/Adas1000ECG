import numpy as np
import csv
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
fig, axs = plt.subplots(3,1,sharex='col')

def animate(i):
    decData = []
    rawdata = (pd.read_csv('data.csv' , usecols = ['t', 'LEAD1/LA','LEAD2/LL','LEAD3/RA']))
    #rawdata = list(csv.reader(f,delimiter = " "))
    t = rawdata['t']
    lead1 = rawdata['LEAD1/LA']
    lead2 = rawdata['LEAD2/LL']
    lead3 = rawdata['LEAD3/RA']
    
    lead1 = lead1.to_numpy()
    lead2 = lead2.to_numpy()
    lead3 = lead3.to_numpy()
    
    for i in lead1:
        decData.append(int(i,16) & 0x00FFFFFF)
    lead1 = decData
    decData = []
    for i in lead2:
        decData.append(int(i,16) & 0x00FFFFFF)
    lead2 = decData
    decData = []
    for i in lead3:
        decData.append(int(i,16) & 0x00FFFFFF)
    lead3 = decData
    plt.cla()
    axs[0].set_title('LEAD1')
    axs[0].plot(t,lead1, "b-")
    axs[1].set_title('LEAD2')
    axs[1].plot(t,lead2, "r-")
    axs[2].set_title('LEAD3')
    axs[2].plot(t,lead3, "g-")
    plt.tight_layout()
    
decData = []
rawdata = (pd.read_csv('data.csv' , usecols = ['t', 'LEAD1/LA','LEAD2/LL','LEAD3/RA']))
#rawdata = list(csv.reader(f,delimiter = " "))
t = rawdata['t']
lead1 = rawdata['LEAD1/LA']
lead2 = rawdata['LEAD2/LL']
lead3 = rawdata['LEAD3/RA']
    
lead1 = lead1.to_numpy()
lead2 = lead2.to_numpy()
lead3 = lead3.to_numpy()
    
for i in lead1:
    decData.append(int(i,16) & 0x00FFFFFF)
lead1 = decData
decData = []
for i in lead2:
    decData.append(int(i,16) & 0x00FFFFFF)
lead2 = decData
decData = []
for i in lead3:
    decData.append(int(i,16) & 0x00FFFFFF)
lead3 = decData
plt.cla()
axs[0].set_title('LEAD1')
axs[0].plot(t,lead1, "b-")
axs[1].set_title('LEAD2')
axs[1].plot(t,lead2, "r-")
axs[2].set_title('LEAD3')
axs[2].plot(t,lead3, "g-")
plt.tight_layout()


#ani = animation.FuncAnimation(fig, animate, interval = 10)

plt.tight_layout()
plt.show()
