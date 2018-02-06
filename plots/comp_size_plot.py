import sys
import psycopg2
from psycopg2.extensions import AsIs
import matplotlib.pyplot as plt
from math import floor
import matplotlib.patches as mpatches
from cycler import cycler

db = sys.argv[1]
table_e = 'cleaned_ways'
table_v = 'cleaned_ways_vertices_pgr'
scc_table = 'scc_ways'
d = {}
d["db"] = db
d["table_e"] = table_e
d["table_v"] = table_v
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="10.2.16.78", port="5432")
d['conn'] = conn
cur = conn.cursor()
num_levels = int(sys.argv[2])
width = 0.43
comp_query = "SELECT min(size), avg(size), max(size) FROM (SELECT count(*) AS size FROM %s WHERE component_%s != 1 GROUP BY abs(component_%s)) AS foo"
size_query = "SELECT count(*) FROM %s"


#Fetching number of on contracted edges
cur.execute(size_query, (AsIs(scc_table), ));
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
                #print "row: ", row[0]
                #min_sizes.append(float(row[0])*100.00/(E*1.00));
                avg_sizes.append(float(row[1])*100.00/(E*1.00));
                max_sizes.append(float(row[2])*100.00/(E*1.00));


fig, ax = plt.subplots(1,1)

ax.grid(axis='y', markersize=10, linewidth='1', linestyle='--')  
ax.set_axisbelow(True)
bar_cycle = (cycler('hatch', ['\\\\']) * cycler('color', 'w')*cycler('zorder', [10]))
styles = bar_cycle()

x = 0
gap = 0.23
x_ticks = []
for i in levels:
        '''
        plt.bar(pos, 
        #using df['pre_score'] data,
        min_sizes, 
        # of width
        width, 
        # with alpha 0.5
        alpha=0.55, 
        # with color
        #color='#FFC222', 
        # with label the first value in first_name
        label=i,
        **next(styles)) 
        '''
        p1 = x+0.001
        plt.bar(p1, 
        #using df['mid_score'] data,
        avg_sizes[i-1],
        # of width
        width,
        linewidth=1.1,
        # with alpha 0.5
        #alpha=0.75, 
        # with color
        color='0', 
        edgecolor='0',
        # with label the second value in first_name
        label=i)


        plt.bar(p1+width, 
        #using df['post_score'] data,
        max_sizes[i-1], 
        # of width
        width, 

        linewidth=1.1,
        edgecolor='0',
        # with alpha 0.5
        #alpha=0.75, 
        # with color
        #        color='#EE3224', 
        # with label the third value in first_name
        label=i, **next(styles)) 
        x_ticks.append(p1+width)
        x = p1 + 2*width + gap 
 
## Set the y axis label
ax.set_ylabel('Size of zones(%)', fontsize=20)
ax.set_xlabel('$\it{i}$', fontsize=20)
# Set the chart's title
#ax.set_title('Component sizes for '+db)

# Set the tick labels font
for label in (ax.get_xticklabels() + ax.get_yticklabels()):
    label.set_fontsize(20)

# Set the position of the x ticks
ax.set_xticks(x_ticks)

# Set the labels for the x ticks
ax.set_xticklabels(levels)

ax.xaxis.set_tick_params(length=1)

# Setting the x-axis and y-axis limits
plt.xlim(min(pos)-width, max(pos)+width*4)
plt.ylim([0, max(max_sizes)*1.01])

# Adding the legend and showing the plot
plt.legend(['Avg', 'Max'], loc='upper right', prop={'size': 26})
#plt.grid(axis='y', markersize=10, linewidth='0.5')
#plt.show()
plt.tight_layout()
plt.savefig('./images/'+db+'_comp_size_'+str(num_levels)+'.png', facecolor='white')
