# -*- coding: utf-8 -*-
"""
Created on Wed Oct 26 09:21:37 2016

@author: rohithreddy
"""

import psycopg2
import math
import sys
table_name = "cleaned_ways"
db_name= sys.argv[1]
n_cells = int(sys.argv[2])

conn = psycopg2.connect(database=db_name, user="postgres", password="postgres", host="127.0.0.1", port="5432")

cur = conn.cursor()
bbox_query = "select \
ST_X(st_setsrid(st_makepoint(st_xmin(st_extent(the_geom)), st_ymin(st_extent(the_geom))), 4326)), \
ST_Y(st_setsrid(st_makepoint(st_xmin(st_extent(the_geom)), st_ymin(st_extent(the_geom))), 4326)), \
ST_X(st_setsrid(st_makepoint(st_xmax(st_extent(the_geom)), st_ymax(st_extent(the_geom))), 4326)),  \
ST_Y(st_setsrid(st_makepoint(st_xmax(st_extent(the_geom)), st_ymax(st_extent(the_geom))), 4326)) \
from " + table_name;


cur.execute(bbox_query)
rows = cur.fetchall()
for row in rows:
    bbox = row
sw = [bbox[0], bbox[1]]
ne = [bbox[2], bbox[3]]
x_min = sw[0]
y_min = sw[1]
x_max = ne[0]
y_max = ne[1]

#print "y_max: " , y_max
#print "SW: " + str(sw)
#print "NE: " + str(ne)

sw_text = 'POINT('+str(x_min)+' '+str(y_min)+')'; 
nw_text = 'POINT('+str(x_min)+' '+str(y_max)+')';
se_text = 'POINT('+str(x_max)+' '+str(y_min)+')';


"""
Fetching length and breadth in 4326 projection
Used for getting square bounding box coordinates

"""

length = x_max - x_min
breadth = y_max - y_min

print "length: " + str(length);
print "breadth: " + str(breadth);


"""
Getting the max of length and breadth to decide the size of the square
"""
if length > breadth:
    sq_size = length;
else:
    sq_size = breadth;


"""
Rectangular bounding box geometry
"""
rect_bbox_query = "SELECT ST_AsText(ST_Extent(the_geom)) FROM "+table_name;
cur.execute(rect_bbox_query);
rows = cur.fetchall()
for r in rows:
    ngeom = r[0];
    print "geom: "+str(ngeom)

ins_bbox_query =  "INSERT INTO bounding_boxes(id, the_geom) VALUES (10, ST_GeomFromText('"+str(ngeom)+"', 4326));";
cur.execute(ins_bbox_query);
conn.commit();

"""
Expanding the rectangular bounding box to get
a square bounding box
"""
geom_query = "(SELECT ST_Extent(the_geom) FROM "+table_name+")";
count = 2;
diff = abs(length-breadth)
    

if length < breadth:
    sq_bbox_query = "SELECT  ST_AsText(ST_Expand(ST_Extent(the_geom) , "+str(diff/2)+"::float4, 0.0::float4)) FROM "+table_name;
else:
    sq_bbox_query = "SELECT  ST_AsText(ST_Expand(ST_Extent(the_geom) , 0.0::float4, "+str(diff/2)+"::float4)) FROM "+table_name;    
cur.execute(sq_bbox_query)
rows = cur.fetchall()
for row in rows:
    #print str(row[0])
    square_geom = row[0];
ins_bbox_query =  "INSERT INTO bounding_boxes(id, the_geom) VALUES (0, ST_GeomFromText('"+str(square_geom)+"', 4326));";
cur.execute(ins_bbox_query)
conn.commit();
"""
Getting SW of the square bounding box
"""
sq_sw_q1 = "SELECT ST_XMin(ST_GeomFromText('"+square_geom+"'))";
sq_sw_q2 = "SELECT ST_YMin(ST_GeomFromText('"+square_geom+"'))";
cur.execute(sq_sw_q1)
rows = cur.fetchall()
for row in rows:
    sq_sw_X = row[0];
cur.execute(sq_sw_q2)
rows = cur.fetchall()
for row in rows:
    sq_sw_Y = row[0];

"""
Getting borders of the square bounding box
"""
sq_sw_q = "SELECT ST_AsText(ST_MakePoint(ST_XMin(ST_GeomFromText('"+square_geom+"')), ST_YMin(ST_GeomFromText('"+square_geom+"'))))";
sq_se_q = "SELECT ST_AsText(ST_MakePoint(ST_XMax(ST_GeomFromText('"+square_geom+"')), ST_YMin(ST_GeomFromText('"+square_geom+"'))))";
sq_nw_q = "SELECT ST_AsText(ST_MakePoint(ST_XMin(ST_GeomFromText('"+square_geom+"')), ST_YMax(ST_GeomFromText('"+square_geom+"'))))";
sq_ne_q = "SELECT ST_AsText(ST_MakePoint(ST_XMax(ST_GeomFromText('"+square_geom+"')), ST_YMax(ST_GeomFromText('"+square_geom+"'))))";
cur.execute(sq_sw_q)
rows = cur.fetchall()
for row in rows:
    sq_sw_text = row[0];

cur.execute(sq_nw_q)
rows = cur.fetchall()
for row in rows:
    sq_nw_text = row[0];

cur.execute(sq_se_q)
rows = cur.fetchall()
for row in rows:
    sq_se_text = row[0];

cur.execute(sq_ne_q)
rows = cur.fetchall()
for row in rows:
    sq_ne_text = row[0];

length_query = "SELECT ST_Distance(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_se_text+"')) ;"
breadth_query = "SELECT ST_Distance(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_nw_text+"')) ;"

cur.execute(length_query)
rows = cur.fetchall()
for row in rows:
    length = row
length = float(length[0])

cur.execute(breadth_query)
rows = cur.fetchall()
for row in rows:
    breadth = row
breadth = float(breadth[0])

print "length: ", length
print "breadth: ", breadth

if length > breadth:
    sq_size = length
else:
    sq_size = breadth

grid_query = "SELECT * FROM RegularGridXY(\
            ST_X(ST_GeomFromText('"+sq_sw_text+"'))::numeric , ST_Y(ST_GeomFromText('"+sq_sw_text+"'))::numeric, \
            ST_X(ST_GeomFromText('"+sq_ne_text+"'))::numeric , ST_Y(ST_GeomFromText('"+sq_ne_text+"'))::numeric \
            ,"+str(sq_size/float(n_cells))+", "+str(sq_size/float(n_cells))+", \
            "+str(n_cells)+", "+str(n_cells)+", 4326)";

#print grid_query

cur.execute(grid_query);
rows = cur.fetchall()
for r in rows:
    i_query = "INSERT INTO grids(x, y, the_geom)\
    VALUES ("+str(r[1])+"::int, "+str(r[0])+"::int, '"+str(r[2])+"');";
    cur.execute(i_query);
conn.commit();

populate_vertices_query = "SELECT * FROM ST_PopulateVertices()";
cur.execute(populate_vertices_query);
conn.commit();

