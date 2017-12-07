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
num_levels = 10
width = 0.25
comp_query = "SELECT min(size), avg(size), max(size) FROM (SELECT count(*) AS size FROM %s WHERE component_%s != 1 GROUP BY component_%s) AS foo"
size_query = "SELECT count(*) FROM %s"


#Fetching number of on contracted edges
cur.execute(size_query, (AsIs(table_e), ));
rows = cur.fetchall();
for row in rows:
        E = float(row[0])
'''
#Fetching number of on contracted edges
cur.execute(size_query, (AsIs('contracted_ways'), ));
rows = cur.fetchall();
for row in rows:
    E += float(row[0])
'''
print 'total_edges: ' + str(E)

levels = [x+1 for x in range(num_levels-1)]
pos = range(num_levels-1)
min_sizes = []
max_sizes = []
avg_sizes = []

for i in levels:
        cur.execute(comp_query, (AsIs(table_e), i, i));
        rows = cur.fetchall();
        for row in rows:
                min_sizes.append(float(row[0])*100.00/(E*1.00));
                avg_sizes.append(float(row[1])*100.00/(E*1.00));
                max_sizes.append(float(row[2])*100.00/(E*1.00));


fig, ax = plt.subplots(figsize=(10,num_levels/2))

for i in levels:
        plt.bar(pos, 
        #using df['pre_score'] data,
        min_sizes, 
        # of width
        width, 
        # with alpha 0.5
        alpha=0.5, 
        # with color
                color='#FFC222', 
        # with label the first value in first_name
        label=i) 


        plt.bar([p + width for p in pos], 
        #using df['mid_score'] data,
        avg_sizes,
        # of width
        width, 
        # with alpha 0.5
        alpha=0.5, 
        # with color
        color='#F78F1E', 
        # with label the second value in first_name
        label=i)


        plt.bar([p + width*2 for p in pos], 
        #using df['post_score'] data,
        max_sizes, 
        # of width
        width, 
        # with alpha 0.5
        alpha=0.5, 
        # with color
                color='#EE3224', 
        # with label the third value in first_name
        label=i) 
 
# Set the y axis label
ax.set_ylabel('%')

# Set the chart's title
ax.set_title('Component sizes for '+db)

# Set the position of the x ticks
ax.set_xticks([p + 1.5 * width for p in pos])

# Set the labels for the x ticks
ax.set_xticklabels(levels)

# Setting the x-axis and y-axis limits
plt.xlim(min(pos)-width, max(pos)+width*4)
plt.ylim([0, max(max_sizes)+5])

# Adding the legend and showing the plot
plt.legend(['Min', 'Avg', 'Max'], loc='upper right')
plt.grid()
#plt.show()
plt.savefig('./images/'+db+'_comp_size.png',facecolor='white')
