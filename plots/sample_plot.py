from sklearn import datasets
from matplotlib import pyplot as plt
import numpy as np
from matplotlib import lines, markers
from cycler import cycler

'''
bar_cycle = (cycler('hatch', ['///', '--', '...','\///', 'xxx', '\\\\']) * cycler('color', 'w')*cycler('zorder', [10]))
styles = bar_cycle()

for x in range(1,5):
    ax.bar(x, np.random.randint(2,10), **next(styles))

plt.show()
'''
# Create cycler object. Use any styling from above you please
monochrome = (cycler('color', ['k']) * cycler('linestyle', ['--', '-', '-', '-', '-', '-']) + cycler('marker', ['', 's','o', 'x', '^', 'D']))

# Print examples of output from cycler object. 
# A cycler object, when called, returns a `iter.cycle` object that iterates over items indefinitely
print("number of items in monochrome:", len(monochrome))
#for i, item in zip(range(15), monochrome()):
#    print(i, item)

fig, ax = plt.subplots(1,1)
ax.set_prop_cycle(monochrome)
for i in range(1,7):
    ax.plot(np.arange(10), np.arange(10)*i, linewidth=1,markerfacecolor='None',label=i-1)
ax.legend()

plt.show()