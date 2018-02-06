import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
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
num_levels = int(sys.argv[2])
width = 0.55
count_query = "SELECT count(distinct(abs(component_%s))) FROM %s";

levels = [x+1 for x in range(num_levels)]
pos = range(num_levels)
skeleton_sizes = []

for i in levels:
        cur.execute(count_query, (i, AsIs(table_e)));
        rows = cur.fetchall();
        for row in rows:
                skeleton_sizes.append(row[0]);


fig, ax = plt.subplots()

ax.grid(axis='y', markersize=10, linewidth='1', linestyle='--')  
ax.set_axisbelow(True)
for i in levels:
        plt.bar(pos, 
        #using df['pre_score'] data,
        skeleton_sizes, 
        # of width
        width, 
        # with alpha 0.5
        #alpha=0.55, 
        # with color
        color='0', 
        # with label the first value in first_name
        label=i) 

# Set the y axis label
ax.set_ylabel('Number of zones', fontsize=20)
ax.set_xlabel('$\it{i}$', fontsize=20)
# Set the chart's title
#ax.set_title('Number of components for '+db)


for tick in ax.xaxis.get_minor_ticks():
    tick.tick1line.set_markersize(0)
    tick.tick2line.set_markersize(0)
    tick.label1.set_horizontalalignment('center')

# Set the tick labels font
for label in (ax.get_xticklabels() + ax.get_yticklabels()):
    label.set_fontsize(20)

# Set the position of the x ticks
ax.set_xticks([p+1 - 1.7*width for p in pos])


# Set the labels for the x ticks
ax.set_xticklabels(levels)


ax.xaxis.set_tick_params(length=1)

# Setting the x-axis and y-axis limits
plt.xlim(min(pos)-width, max(pos)+width*2)
plt.ylim([0, max(skeleton_sizes)*1.01])

# Adding the legend and showing the plot
#plt.grid(axis='y', markersize=10, linewidth='0.5', markeredgewidth=0.9)
#plt.show()
plt.tight_layout()
plt.savefig('./images/'+db+'_comp_count_'+str(num_levels)+'.png',facecolor='white')
