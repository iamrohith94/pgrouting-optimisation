import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
import numpy as np
from matplotlib import lines, markers
from cycler import cycler
db = sys.argv[1]
table_e = 'cleaned_ways'
table_v = 'cleaned_ways_vertices_pgr'
performance_table = 'performance_analysis'
d = {}
d["db"] = db
d["table_e"] = table_e
d["table_v"] = table_v
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="localhost", port="5432")
d['conn'] = conn
cur = conn.cursor()
num_levels = 10
num_intervals = 5
width = 0.15

levels = [10, 2, 3, 4, 6]


monochrome = (cycler('color', ['k']) * cycler('linestyle', ['--', '-', '-', '-', '-', '-']) + cycler('marker', ['', 's','o', 'x', '^', 'D' ]))

avg_computation_time = []

avg_time_query = "SELECT qset, avg(avg_computation_time) FROM %s WHERE level = %s GROUP BY qset ORDER BY qset"


fig, ax = plt.subplots(1,1)
ax.set_prop_cycle(monochrome)
for level in levels:
    avg_computation_time = []
    qset = []
    cur.execute(avg_time_query, (AsIs(performance_table), level,));
    rows = cur.fetchall();
    for row in rows:
        qset.append(row[0])
        avg_computation_time.append(row[1])
    ax.plot(qset, avg_computation_time, linewidth=1,markerfacecolor='None',label=level)
ax.legend(prop={'size': 15})


ax.set_xticks(qset)

# Set the labels for the x ticks
ax.set_xticklabels(qset)


for label in (ax.get_xticklabels() + ax.get_yticklabels()):
    label.set_fontsize(20)


## Set the y axis label
ax.set_ylabel('RunTime(milli sec)', fontsize=20)
ax.set_xlabel('$\it{Q}$', fontsize=20)

ax.xaxis.set_tick_params(length=1)

plt.tight_layout()
plt.savefig('./images/'+db+'_comp_time_'+str(num_levels)+'.png', facecolor='white')