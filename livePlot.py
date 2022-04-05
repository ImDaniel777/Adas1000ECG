import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import csv
from itertools import count
import numpy as np
import pandas as pd
style.use('fivethirtyeight')

fig, axs = plt.subplots(3,1,sharex='col')

def animate(i):
    rawdata = pd.read_csv('data.csv' , usecols = ['t','LEAD1/LA','LEAD2/LL','LEAD3/RA'])
    t = rawdata['t']
    lead1 = rawdata['LEAD1/LA']
    lead2 = rawdata['LEAD2/LL']
    lead3 = rawdata['LEAD3/RA']

    lead1 = lead1.to_numpy()
    lead2 = lead2.to_numpy()
    lead3 = lead3.to_numpy()

    decData = []
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
    axs.clear()
    axs[0].set_title('LEAD1')
    axs[0].plot(time,lead1)
    axs[1].set_title('LEAD2')
    axs[1].plot(time,lead2)
    axs[2].set_title('LEAD3')
    axs[2].plot(time,lead3)

ani = animation.FuncAnimation(fig, animate, interval = 1000)
plt.show()
    