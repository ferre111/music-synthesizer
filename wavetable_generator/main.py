import math as m
import matplotlib.pyplot as plt

def saw(length):
    x = []
    for i in range(int(-length/2), int(length/2)):
        x.append(round(i * 32767 / 24000))
    return x

def triangle(length):
    x = []
    section = length // 4
    for direction in (1, -1):
        for i in range(section):
            x.append(round(i * (32767 / section) * direction))
        for i in range(section):
            x.append(round((32767 - (i * (32767 / section))) * direction))
    return x


def squre(length):
    x = []
    for i in range(length):
        if i < int(length/2):
            val = 32767
        else:
            val = -32767
        x.append(val)
    return x

def sin(length):
    x = []
    for i in range(0, int(fs * 1 / f)):
        x.append(round(32767 * m.sin(2 * m.pi * f * i / fs)))
    return x

fs = 48000
f = 1
out = []
x = []
sample = []

for i in range(0, int(fs*1/f)):
    x.append(i)

out = saw(48000)

print(len(out))
print(out)

plt.plot(out, 'r.')
plt.grid()
plt.show()

for i in range(0, 3):
    print(m.sin(2.0 * m.pi * 1000 * i/48000))
