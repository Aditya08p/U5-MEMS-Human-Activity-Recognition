import argparse
import numpy as np
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument('filename')
args = parser.parse_args()

with open(args.filename) as f:
    first_line = f.readline()

has_header = not first_line[0].isdigit()

data = np.loadtxt(args.filename, delimiter=',', skiprows=1 if has_header else 0)

if data.ndim == 1:
    data = data.reshape(-1, 1)

labels = first_line.strip().split(',') if has_header else [f'col_{i}' for i in range(data.shape[1])]

# Extract timestamp
time = data[:, 0] / 1000.0   # ms → seconds
signals = data[:, 1:]

plt.figure(figsize=(10, 3))

for i in range(signals.shape[1]):
    plt.plot(time, signals[:, i], label=labels[i+1])

plt.xlabel("Time (s)")
plt.legend()
plt.show()