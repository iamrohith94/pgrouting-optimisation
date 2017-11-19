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
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="127.0.0.1", port="5432")
d['conn'] = conn
cur = conn.cursor()
num_levels = 10
width = 0.25
skeleton_query = "SELECT count(*) FROM %s WHERE promoted_level <= %s";
size_query = "SELECT count(*) FROM %s"


#Fetching total number of edges
cur.execute(size_query, (AsIs(table_e), ));
rows = cur.fetchall();
for row in rows:
	E = float(row[0])

levels = [x+1 for x in range(num_levels-1)]
pos = range(num_levels-1)
skeleton_sizes = []

for i in levels:
	cur.execute(skeleton_query, (AsIs(table_e), i));
	rows = cur.fetchall();
	for row in rows:
		skeleton_sizes.append(float(row[0])*100.00/(E*1.00));


fig, ax = plt.subplots(figsize=(10,num_levels/2))

for i in levels:
	plt.bar(pos, 
        #using df['pre_score'] data,
        skeleton_sizes, 
        # of width
        width, 
        # with alpha 0.5
        alpha=0.2, 
        # with color
		color='#FFC222', 
        # with label the first value in first_name
        label=i) 

# Set the y axis label
ax.set_ylabel('%')

# Set the chart's title
ax.set_title('Skeleton sizes for '+db)

# Set the position of the x ticks
ax.set_xticks([p + width for p in pos])

# Set the labels for the x ticks
ax.set_xticklabels(levels)

# Setting the x-axis and y-axis limits
plt.xlim(min(pos)-width, max(pos)+width*2)
plt.ylim([0, max(skeleton_sizes)+5])

# Adding the legend and showing the plot
plt.grid()
#plt.show()
plt.savefig('images/'+db+'_skeleton_size.png',facecolor='white')
