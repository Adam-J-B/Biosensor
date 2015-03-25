from serial import Serial
from serial.tools import list_ports
import os
import datetime

baud = 9600

#Scan USB ports, and find the serial connection to the Arduino
port_list = list_ports.comports()
for line in port_list:
	if('usb' in line[2].lower()):
		chosenPort = line[0]
ser = Serial(chosenPort,baud)

dt=str(datetime.datetime.now().strftime("%y-%m-%d_%H%M"))

myfile = open(dt + ".txt", "w")

iteration = 0
while(True):
    l = ser.readline()
    print(l)
    myfile.write(l)
    iteration += 1
    if(iteration % 12 == 0):
        myfile.flush()
        os.fsync(myfile.fileno())
