import matplotlib.pyplot as plt
from psycopg2.extensions import AsIs
import psycopg2
import sys

def betweenness_distribution(parameters):
	"""
	Draws a plot between the edge_count(betweenness) 
	VS the number of edges(having a particular betweenness)
	"""
	conn = parameters['conn']
	cur = conn.cursor()
	edge_count = []
	betweenness_values = []
	query = "SELECT count(*), betweenness FROM %s GROUP BY betweenness"
	cur.execute(query, (AsIs(parameters['table_e']),) )
	rows = cur.fetchall()
	for row in rows:
		edge_count.append(row[0])
		betweenness_values.append(row[1])
	plt.xlabel('Betweenness')
	plt.ylabel('Edge count')
	plt.plot(betweenness_values, edge_count, 'ro')
	plt.savefig('./images/'+parameters['db']+'_betweenness_distribution.png',facecolor='white')

db = sys.argv[1]
table_e = 'cleaned_ways'
table_v = 'cleaned_ways_vertices_pgr'

d = {}
d["db"] = db
d["table_e"] = table_e
d["table_v"] = table_v
conn = psycopg2.connect(database=d['db'], user="postgres", password="postgres", host="10.2.16.78", port="5432")
d['conn'] = conn

betweenness_distribution(d)
