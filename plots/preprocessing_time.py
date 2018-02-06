import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
from cycler import cycler

datasets = ['CHD', 'HYD', 'NYC', 'BLG']

'''
The below data is for 16gb linux machine
for 8 parallel processes
edge importance and connected skeleton were parallelized
zone generation is sequential
'''
edge_importance = [0.22, 11.65, 31.9, 58]
connected_skeleton = [1.75, 154.45, 573.4, 3044.9]
zone_generation = [0.33, 2.7, 30.7, 319.4]
fig, ax = plt.subplots(1,1)


bar_cycle = (cycler('hatch', ['...','\\\\']) * cycler('color', 'w')*cycler('zorder', [10]))
styles = bar_cycle()

width = 0.15

pos = [0.3+x*0.4 for x in range(len(datasets))]

edge_importance = [e*1.00/2 for e in edge_importance]
connected_skeleton = [c*1.00/2 for c in connected_skeleton]



max_values=[]
temp=[]
for i in range(len(datasets)):
	temp.append(edge_importance[i]+connected_skeleton[i])
	max_values.append(edge_importance[i]+connected_skeleton[i]+zone_generation[i])
#for dataset in datasets:
plt.bar(pos, 
    #using df['pre_score'] data,
    edge_importance, 
    # of width
    width, 
    # with alpha 0.5
    #alpha=0.55, 
    # with color
    color='0', 
    # with label the first value in first_name
    ) 

plt.bar(pos, 
    #using df['pre_score'] data,
    connected_skeleton, 
    # of width
    width,
    bottom = edge_importance, 
    # with alpha 0.5
    #alpha=0.55, 
    # with color
    **next(styles))

plt.bar(pos, 
    #using df['pre_score'] data,
    zone_generation, 
    # of width
    width, 
    bottom = temp,	
    # with alpha 0.5
    #alpha=0.55, 
    # with color
    **next(styles))

ax.set_ylabel('Runtime(min)', fontsize=20)
ax.set_xlabel('Dataset', fontsize=20)


for tick in ax.xaxis.get_minor_ticks():
    tick.tick1line.set_markersize(0)
    tick.tick2line.set_markersize(0)
    tick.label1.set_horizontalalignment('center')

for label in (ax.get_xticklabels() + ax.get_yticklabels()):
    label.set_fontsize(20)

# Set the position of the x ticks
ax.set_xticks([p+width*0.5 for p in pos])

# Set the labels for the x ticks

ax.xaxis.set_tick_params(length=1)


ax.set_xticklabels(datasets)

# Setting the x-axis and y-axis limits
#plt.xlim(min(pos)-width, max(pos)+width*4)
plt.ylim([0, max(max_values)*1.5])

# Adding the legend and showing the plot
plt.legend(['Edge Importance', 'Skeleton Generation', 'Zone Generation'], loc='upper right', prop={'size': 20})
plt.grid(axis='y', markersize=10, linewidth='0.5')
#plt.show()
plt.tight_layout()
plt.savefig('./images/preprocess_time.png', facecolor='white')


'''
SELECT path, ST_AsText(geom)
FROM (
  SELECT (ST_DumpPoints(g.geom)).*
  FROM
    (SELECT st_collect(the_geom) from cleaned_ways_vertices_pgr
    ) AS g
  ) j;
 '''