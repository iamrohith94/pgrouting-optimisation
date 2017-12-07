import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
import numpy as np
db = sys.argv[1]
table_e = 'cleaned_ways'
table_v = 'cleaned_ways_vertices_pgr'

d = {}
d["db"] = db
d["table_e"] = table_e
d["table_v"] = table_v
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="10.2.16.78", port="5432")
d['conn'] = conn
cur = conn.cursor()
num_levels = 10
num_intervals = 5
width = 0.15
actual_dist_query = "SELECT source, target, path_len FROM performance_results WHERE level = 10 ORDER BY path_len"
appx_time_query = "SELECT avg_computation_time FROM performance_results WHERE level = %s AND source = %s AND target = %s"

actual_distances = []
#Fetching number of on contracted edges

st_pairs = []
cur.execute(actual_dist_query);
rows = cur.fetchall();
for row in rows:
        st_pairs.append((row[0], row[1]))
        actual_distances.append(row[2])
min_actual_dist = min(actual_distances)*111
max_actual_dist = max(actual_distances)*111


#print "st_pairs:"
#print st_pairs

st_ranges = np.array_split(st_pairs, num_intervals)

#print "distance range:"
#print dist_ranges

#print "st_range:"
#print st_ranges


levels = [x+1 for x in range(num_levels-1)]
pos = range(num_levels-1)

avg_time_values = {}


for i in levels:
        avg_time_values[i] = []
        for j in range(0, num_intervals):
                values = []
                for k in range(0, len(st_ranges[j])):
                        s = st_ranges[j][k][0]
                        t = st_ranges[j][k][1]
#                        a_d = dist_ranges[j][k]
                        cur.execute(appx_time_query, (i, s, t,));
                        rows = cur.fetchall();
                        for row in rows:
                                values.append(row[0])
                avg_time_values[i].append(np.mean(values))
        #print len(approx_values[i])
'''
print "approx_values"
for i in levels:
        print i, approx_values[i]
'''

fig, ax = plt.subplots(figsize=(10,num_levels/2))

for i in levels:
        for j in range(0, num_intervals):
                plt.bar([p + width*j for p in pos], 
                #using df['post_score'] data,
                [avg_time_values[l][j] for l in levels],
                # of width
                width, 
                # with alpha 0.5
                alpha=0.3, 
                # with color
                color='#EE3224', 
                # with label the third value in first_name
                label=i) 
 
# Set the y axis label
ax.set_ylabel('Avg Computation Time (millisec)')
ax.set_xlabel('Actual Distance ('+str(format(min_actual_dist, '.2f'))+'kms - '+str(format(max_actual_dist, '.2f'))+'kms)')

# Set the chart's title
ax.set_title('Computation time for '+db)

# Set the position of the x ticks
ax.set_xticks([p + 1.5 * width for p in pos])

# Set the labels for the x ticks
ax.set_xticklabels(levels)

# Setting the x-axis and y-axis limits
#plt.xlim(min(pos)-width, max(pos)+width*4)
#plt.ylim([0, max(max_sizes)+5])

# Adding the legend and showing the plot
plt.grid()
#plt.show()
plt.savefig('./images/'+db+'_path_computation_time.png',facecolor='white')
