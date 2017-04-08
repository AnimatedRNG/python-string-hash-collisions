#!/usr/bin/env python3

import matplotlib.pyplot as plt
from datetime import datetime
from os import popen

timings = [0]

for i in range(1, 32):
    hook = 'Total time elapsed: '
    results = popen('bin/hash_collision {}'.format(i)).read()
    length = float(results[results.find(hook) + len(hook):])
    timings.append(length)
    print("Finished {}".format(i))

plt.plot(timings)
plt.title('Time to find collision')
plt.ylabel('Time in seconds')
plt.xlabel('Common bits')
plt.show()
