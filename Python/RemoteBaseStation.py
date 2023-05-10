import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import socket              
from time import sleep
import struct


fig, ax = plt.subplots()

x = np.arange(0, 100, 1)


global y_data1
global y_data2
global y_data3
global y_data4
global y_data5
global y_data6
global y_data7

y_data1 = np.zeros(len(x))
y_data2 = np.zeros(len(x))
y_data3 = np.zeros(len(x))
y_data4 = np.zeros(len(x))
y_data5 = np.zeros(len(x))
y_data6 = np.zeros(len(x))
y_data7 = np.zeros(len(x))

line1, = ax.plot(x)
line2, = ax.plot(x)
line3, = ax.plot(x)
line4, = ax.plot(x)
line5, = ax.plot(x)
line6, = ax.plot(x)
line7, = ax.plot(x)



s = socket.socket()        
host = '192.168.5.1' 
port = 8888

print("trying to connect")
s.connect((host, port))
print("connected!")

data_floats = [0.0, 1.1]
data_bytes = struct.pack('<2f', *data_floats)
print(data_bytes);
s.send(data_bytes)


def animate(i):
    global y_data1
    global y_data2
    global y_data3
    global y_data4
    global y_data5
    global y_data6
    global y_data7
    
    data_floats = [0.0, 1.1]
    data_bytes = struct.pack('<2f', *data_floats)
    s.send(data_bytes)
    all_data_raw = s.recv(28)
    all_data = struct.unpack('<7f', all_data_raw)

    # shift arrays left by one
    y_data1 = np.roll(y_data1, -1);
    y_data2 = np.roll(y_data2, -1);
    y_data3 = np.roll(y_data3, -1);
    y_data4 = np.roll(y_data4, -1);
    y_data5 = np.roll(y_data5, -1);
    y_data6 = np.roll(y_data6, -1);
    y_data7 = np.roll(y_data7, -1);

    # change the last value with the most recent
    y_data1[-1] = all_data[0];
    y_data2[-1] = all_data[1];
    y_data3[-1] = all_data[2];
    y_data4[-1] = all_data[3];
    y_data5[-1] = all_data[4];
    y_data6[-1] = all_data[5];
    y_data7[-1] = all_data[6];

    # update the data
    line1.set_ydata(y_data1)  
    line2.set_ydata(y_data2)  
    line3.set_ydata(y_data3)  
    line4.set_ydata(y_data4)  
    line5.set_ydata(y_data5)  
    line6.set_ydata(y_data6)  
    line7.set_ydata(y_data7)  
    
    return [line1, line2, line3, line4, line5, line6, line7, ]

ani = animation.FuncAnimation( fig, animate, interval=0.001, blit=True, save_count=1)
plt.show()


# To save the animation, use e.g.
#
# ani.save("movie.mp4")
#
# or
#
# writer = animation.FFMpegWriter(
#     fps=15, metadata=dict(artist='Me'), bitrate=1800)
# ani.save("movie.mp4", writer=writer)


