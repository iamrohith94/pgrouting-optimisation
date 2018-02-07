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
n_cells = sys.argv[2]

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

print "y_max: " , y_max
#print "SW: " + str(sw)
#print "NE: " + str(ne)

sw_text = 'POINT('+str(x_min)+' '+str(y_min)+')'; 
nw_text = 'POINT('+str(x_min)+' '+str(y_max)+')';
se_text = 'POINT('+str(x_max)+' '+str(y_min)+')';


"""
Fetching length and breadth in 4326 projection
Used for getting square bounding box coordinates

"""
'''
length_query = "SELECT ST_Distance( \
    st_makepoint(" + str(x_min) + ", " + str(y_min) + "), \
    st_makepoint(" + str(x_max) + ", " + str(y_min) + "));"

breadth_query = "SELECT ST_Distance( \
    st_makepoint(" + str(x_min) + ", " + str(y_min) + "), \
    st_makepoint(" + str(x_min) + ", " + str(y_max) + "));"

cur.execute(length_query)
rows = cur.fetchall()
for row in rows:
    length = row
length = length[0]

cur.execute(breadth_query)
rows = cur.fetchall()
for row in rows:
    breadth = row
breadth = breadth[0]
'''
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



"""
Fetching length and breadth in 2100 projection in kms
Used for generating grids


length_kms_query = "SELECT ST_Distance(\
            ST_Transform(\
            ST_GeomFromText('"+sw_text+"',4326),2100),\
            ST_Transform(\
            ST_GeomFromText('"+se_text+"',4326),2100)) / 1000.0 ;"

breadth_kms_query = "SELECT ST_Distance(\
            ST_Transform(\
            ST_GeomFromText('"+sw_text+"',4326),2100),\
            ST_Transform(\
            ST_GeomFromText('"+nw_text+"',4326),2100)) / 1000.0 ;"

"""

"""
spheroid = 'SPHEROID["WGS 84",6378137,298.257223563]';
length_kms_query = "SELECT ST_Distance_Spheroid(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_se_text+"'),\
            '"+str(spheroid)+"') / 980.0 ;"

breadth_kms_query = "SELECT ST_Distance_Spheroid(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_nw_text+"'),\
            '"+str(spheroid)+"') / 980.0 ;"
"""
length_kms_query = "SELECT ST_Distance(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_se_text+"')) * 111.0 ;"
breadth_kms_query = "SELECT ST_Distance(\
            ST_GeomFromText('"+sq_sw_text+"'),\
            ST_GeomFromText('"+sq_nw_text+"')) * 111.0 ;"

cur.execute(length_kms_query)
rows = cur.fetchall()
for row in rows:
    length = row
length_kms = length[0]

cur.execute(breadth_kms_query)
rows = cur.fetchall()
for row in rows:
    breadth = row
breadth_kms = breadth[0]

print "length in kms: ", length_kms
print "breadth in kms: ", breadth_kms

if length_kms > breadth_kms:
    sq_size_kms = length_kms
else:
    sq_size_kms = breadth_kms


sq_sw_text = 'POINT('+str(sq_sw_X)+' '+str(sq_sw_Y)+')'; 


grid_query = "SELECT * FROM ST_CreateGrids(\
                '"+sq_sw_text+"', "+str(sq_size_kms)+", "+str(sq_size_kms)+", "+\
                str(n_cells)+", "+str(n_cells)+")";

cur.execute(grid_query);
rows = cur.fetchall()
for r in rows:
    i_query = "INSERT INTO grids(x, y, the_geom)\
    VALUES ("+str(r[1])+"::int, "+str(r[0])+"::int, '"+str(r[2])+"');";
    cur.execute(i_query);
conn.commit();

'''
populate_grids_query = "SELECT * FROM ST_PopulateGrids()";
cur.execute(populate_grids_query);
conn.commit();
'''
populate_vertices_query = "SELECT * FROM ST_PopulateVertices()";
cur.execute(populate_vertices_query);
conn.commit();

