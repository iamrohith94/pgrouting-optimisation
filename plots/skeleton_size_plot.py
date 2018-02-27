import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
db = sys.argv[1]
table_e = 'cleaned_ways'
table_v = 'cleaned_ways_vertices_pgr'
scc_table = 'scc_ways'

d = {}
d["db"] = db
d["table_e"] = table_e
d["table_v"] = table_v
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="localhost", port="5432")
d['conn'] = conn
cur = conn.cursor()
num_levels = int(sys.argv[2])
width = 0.55
skeleton_query = "SELECT count(*) FROM %s WHERE component_%s = 1";
size_query = "SELECT count(*) FROM %s"


#Fetching total number of edges
cur.execute(size_query, (AsIs(scc_table), ));
rows = cur.fetchall();
for row in rows:
        E = float(row[0])
'''
cur.execute(size_query, (AsIs('contracted_ways'), ));
rows = cur.fetchall();
for row in rows:
    E += float(row[0])
'''
print "total edges: " + str(E)

levels = [x+1 for x in range(num_levels)]
pos = range(num_levels)
skeleton_sizes = []

for i in levels:
        cur.execute(skeleton_query, (AsIs(table_e), i));
        rows = cur.fetchall();
        for row in rows:
                skeleton_sizes.append(float(row[0])*100.00/(E*1.00));


fig, ax = plt.subplots(1,1)

ax.grid(axis='y', markersize=10, linewidth='1', linestyle='--')  
ax.set_axisbelow(True)

print skeleton_sizes

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
ax.set_ylabel('Size of skeleton(%)', fontsize=20)

ax.set_xlabel('$\it{i}$', fontsize=20)
# Set the chart's title
#ax.set_title('Skeleton sizes for '+db)




#Center tick labels
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
plt.ylim([0, max(skeleton_sizes)+5])

plt.tight_layout()

# Adding the legend and showing the plot
#plt.show()
plt.savefig('./images/'+db+'_skeleton_size_'+str(num_levels)+'.png',facecolor='white')
