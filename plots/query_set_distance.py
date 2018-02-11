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
width = 0.15

levels = [2, 3, 4, 6]


monochrome = (cycler('color', ['k']) * cycler('linestyle', ['-', '-', '-', '-', '-']) + cycler('marker', ['s','o', 'x', '^', 'D' ]))

actual_distances_map = {}


approx_dist_query = "SELECT qset, path_len FROM %s WHERE level = %s ORDER BY qset, source, target"

cur.execute(approx_dist_query, (AsIs(performance_table), 10,));
rows = cur.fetchall();
for row in rows:
	try:
		actual_distances_map[row[0]].append(row[1])
	except KeyError, e:
		actual_distances_map[row[0]] = [row[1]]


print actual_distances_map.keys() 

fig, ax = plt.subplots(1,1)
ax.set_prop_cycle(monochrome)
for level in levels:
	print level
	approx_distances_map = {}
	avg_deviations = []
	cur.execute(approx_dist_query, (AsIs(performance_table), level,));
	rows = cur.fetchall();
	for row in rows:
		try:
			approx_distances_map[row[0]].append(row[1])
		except KeyError, e:
			approx_distances_map[row[0]] = [row[1]]
	print approx_distances_map
	for k in approx_distances_map.keys():
		diff = [(approx_distances_map[k][j] - actual_distances_map[k][j])*100.00/actual_distances_map[k][j] 
				for j in range(len(approx_distances_map[k]))]
		avg_deviations.append(np.mean(diff))

	ax.plot(approx_distances_map.keys(), 
		avg_deviations, 
		linewidth=1, markerfacecolor='None',label=level)
ax.legend(prop={'size': 15})


ax.set_xticks(approx_distances_map.keys())

# Set the labels for the x ticks
ax.set_xticklabels(approx_distances_map.keys())


for label in (ax.get_xticklabels() + ax.get_yticklabels()):
    label.set_fontsize(20)

## Set the y axis label
ax.set_ylabel('Error in Path(%)', fontsize=20)
ax.set_xlabel('$\it{Q}$', fontsize=20)

ax.xaxis.set_tick_params(length=1)


plt.tight_layout()
plt.savefig('./images/'+db+'_path_error_'+str(num_levels)+'.png', facecolor='white')