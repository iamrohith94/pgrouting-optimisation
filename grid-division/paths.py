import psycopg2
import sys

def get_column_values(parameters):
    db=parameters['db']
    table=parameters['table']
    column = parameters['column']
    ret = []
    conn = psycopg2.connect(database=db, user="postgres", password="postgres", host="127.0.0.1", port="5432")    
    cur = conn.cursor()
    cur.execute("SELECT "+column+" from "+table)
    rows=cur.fetchall()
    for row in rows:
        ret.append(row[0]);
    conn.close();
    return ret

def get_vertices_with_diff_paths(parameters):
    db=parameters['db']
    vertices = parameters['vertices']
    limit = parameters['limit']
    v_map = {};
    conn = psycopg2.connect(database=db, user="postgres", password="postgres", host="127.0.0.1", port="5432")    
    cur = conn.cursor()
    del_query = "DELETE FROM path_differences";
    cur.execute(del_query);
    query = "SELECT ST_LineMerge(ST_Collect(the_geom)) FROM ways WHERE gid = ANY(%s)";
    insert_query = "INSERT INTO path_differences(source, target, \
    actual_path, grid_path) VALUES(%s, %s, %s, %s)";
    for i in vertices[1:limit]:
        for j in vertices[1:limit]:
            if i != j:
                cur.execute("SELECT edge FROM grid_dijkstra_1("+str(i)+", "+str(j)+")");
                rows1=cur.fetchall()
                cur.execute("SELECT edge FROM grid_dijkstra_2("+str(i)+", "+str(j)+")");
                rows2=cur.fetchall()
                path_1 = [x[0] for x in rows1];
                path_2 = [x[0] for x in rows2];
                if len(path_1) != len(path_2) and len(path_2) != 0 and len(path_1) != 0 :
                    print [i, j]
                    rows1 = cur.execute(query, (path_1,));
                    rows1=cur.fetchall()
                    rows2 = cur.execute(query, (path_2,));
                    rows2=cur.fetchall()
                    for x in rows1:
                        geom_1 = x[0];
                    for x in rows2:
                        geom_2 = x[0];
                    cur.execute(insert_query, (i, j, geom_1, geom_2));
    conn.commit();
    conn.close();

parameters = {}
limit = 10;
parameters['db'] = "gachibowli";
parameters['table'] = "ways_vertices_pgr";
parameters['column'] = "id";
vertices = get_column_values(parameters);
vertices = [int(x) for x in vertices];
#print vertices
parameters['vertices'] = vertices;
parameters['limit'] = limit;
diff_vertices = get_vertices_with_diff_paths(parameters);
#print diff_vertices

    