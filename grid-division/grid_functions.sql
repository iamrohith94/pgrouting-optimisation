/*
A function to create a fishernet
*/

CREATE OR REPLACE FUNCTION ST_CreateFishnet(
        nrow INTEGER, ncol INTEGER,
        xsize float8, ysize float8,
        x0 float8 DEFAULT 0, y0 float8 DEFAULT 0,
        OUT "row" INTEGER, OUT col INTEGER,
        OUT geom geometry)
    RETURNS SETOF record AS
$$
SELECT i + 1 AS row, j + 1 AS col, ST_Translate(cell, j * $3 + $5, i * $4 + $6) AS geom
FROM generate_series(0, $1 - 1) AS i,
     generate_series(0, $2 - 1) AS j,
(
SELECT ('POLYGON((0 0, 0 '||$4||', '||$3||' '||$4||', '||$3||' 0,0 0))')::geometry AS cell
) AS foo;
$$ LANGUAGE sql IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION ST_PointAtDist(
		origin TEXT,
		d float8,
        brng float8 DEFAULT 89.54, 
        OUT the_geom TEXT)
    RETURNS TEXT AS
$BODY$
DECLARE
	R float;
	origin_x float;
	origin_y float;
	lat1 float;
	lon1 float;
	lat2 float;
	lon2 float;
BEGIN
	origin_x := (SELECT ST_X(ST_GeomFromText(origin)));
	origin_y := (SELECT ST_Y(ST_GeomFromText(origin))); 
	R := 6378.137;

	--IF brng = 0
	--THEN
		--lat2 := asin( sin(lat1)*cos(d/R) + cos(lat1)*sin(d/R));
		--lon2 := lon1 + atan2(0, cos(d/R)-sin(lat1)*sin(lat2));
	--ELSE 
		--lat2 := asin( sin(lat1)*cos(d/R));
		--lon2 := lon1 + atan2(sin(d/R)*cos(lat1), cos(d/R)-sin(lat1)*sin(lat2));
	--END IF;
	lat1 := radians(origin_y);
	lon1 := radians(origin_x);
	lat2 := asin( sin(lat1)*cos(d/R) + cos(lat1)*sin(d/R)*cos(brng));
	lon2 := lon1 + atan2(sin(brng)*sin(d/R)*cos(lat1), cos(d/R)-sin(lat1)*sin(lat2));
	lat2 := degrees(lat2);
	lon2 := degrees(lon2);
	the_geom := (SELECT ST_AsText(ST_SetSRID(ST_MakePoint(lon2, lat2),4326)));
	--the_geom :=  (select 'POINT (' || lon2 || ' ' || lat2 || ')'::text) ;
END
$BODY$
LANGUAGE plpgsql IMMUTABLE STRICT;

/*
origin: the point with min lat and min lon of the square bounding box
length: length of the square bounding box
breadth: breadth of the square bounding box
nrow: number of rows
ncol: number of columns
*/

CREATE OR REPLACE FUNCTION ST_CreateGrids(
		origin TEXT,
		length float8, breadth float8,
        nrow INTEGER, ncol INTEGER,
        OUT "row" INTEGER, OUT col INTEGER,
        OUT geom geometry)
    RETURNS SETOF record AS
$BODY$
DECLARE
	origin_x float;
	origin_y float;
	unit_grid_l float;
	unit_grid_b float;
	sw TEXT;
	nw TEXT;
	ne TEXT;
	se TEXT;
	sw_x float;
	sw_y float;
	nw_x float;
	nw_y float;
	ne_x float;
	ne_y float;
	se_x float;
	se_y float;
BEGIN	

	origin_x := (SELECT ST_X(ST_GeomFromText($1)));
	origin_y := (SELECT ST_Y(ST_GeomFromText($1)));
	unit_grid_l := $2/$5;
	unit_grid_b := $3/$4;
	FOR i in 0..$4-1 LOOP	
	
		FOR j in 0..$5-1 LOOP
			sw :=  (SELECT ST_PointAtDist($1, i*unit_grid_l*0.85));
			sw :=  (SELECT ST_PointAtDist(sw, j*unit_grid_b, 0));
			nw :=  (SELECT ST_PointAtDist(sw, unit_grid_b, 0));
			ne :=  (SELECT ST_PointAtDist(nw, unit_grid_l));
			se :=  (SELECT ST_PointAtDist(sw, unit_grid_l));
			
			sw_x := (SELECT ST_X(ST_GeomFromText(sw)));
			sw_y := (SELECT ST_Y(ST_GeomFromText(sw)));
			nw_x := (SELECT ST_X(ST_GeomFromText(nw)));
			nw_y := (SELECT ST_Y(ST_GeomFromText(nw)));
			ne_x := (SELECT ST_X(ST_GeomFromText(ne)));
			ne_y := (SELECT ST_Y(ST_GeomFromText(ne)));
			se_x := (SELECT ST_X(ST_GeomFromText(se)));
			se_y := (SELECT ST_Y(ST_GeomFromText(se)));
			
			RETURN QUERY
			SELECT i+1 AS row, j+1 AS column,
			ST_GeomFromText('POLYGON(('||sw_x||' '||sw_y||', '||nw_x||' '||nw_y||', '||ne_x||' '||ne_y||', '||se_x||' '||se_y||', '||sw_x||
			' '||sw_y||'))', 4326) AS geom;

			/*
			'POLYGON(('||sw_x||' '||sw_y||', '||nw_x||' '||nw_y||', '||ne_x||' '||ne_y||', '||se_x||' '||se_y||', '||sw_x||
			' '||sw_y||'))' AS geom;
			*/
		END LOOP;
		
	END LOOP;
		
END
$BODY$
LANGUAGE plpgsql IMMUTABLE STRICT;


/*
origin: the point with min lat and min lon of the square bounding box
length: length of the square bounding box
breadth: breadth of the square bounding box
nrow: number of rows
ncol: number of columns
*/

CREATE OR REPLACE FUNCTION ST_PopulateGrids()
    RETURNS void AS
$BODY$
DECLARE
within BOOLEAN;
intersects BOOLEAN;	
edge_id INTEGER;
edge_geom geometry;
v_id INTEGER;
v_geom geometry;
grid_id INTEGER;	
BEGIN	

	
	/*
	Populating edge table with edges that are within
	the grid and also those with intersects the grid
	*/
	FOR edge_id, edge_geom IN
		SELECT gid as edge_id, the_geom as edge_geom
		FROM cleaned_ways
	LOOP
		FOR grid_id, within, intersects IN
			SELECT id,
			ST_Within(edge_geom, the_geom),
			ST_Intersects(edge_geom, the_geom)
			FROM grids
		LOOP
			IF within = TRUE OR intersects = TRUE THEN
				UPDATE ways
				SET grids = grids || grid_id WHERE gid = edge_id;
				UPDATE ways
				SET is_within = within WHERE gid = edge_id;
				IF within = TRUE THEN
					UPDATE grids
					SET within_edges = within_edges || edge_id
					WHERE id = grid_id;
				END IF;
				IF intersects = TRUE THEN
					UPDATE grids
					SET intersect_edges = intersect_edges || edge_id
					WHERE id = grid_id;
				END IF;
			END IF;
		END LOOP;
	END LOOP;

	/*
	Populating vertex table with vertices that are within
	the grid
	*/
	FOR v_id, v_geom IN
		SELECT id as v_id, the_geom as v_geom
		FROM cleaned_ways_vertices_pgr
	LOOP
		FOR grid_id, within IN
			SELECT id,
			ST_Within(v_geom, the_geom)
			FROM grids
		LOOP
			IF within = TRUE THEN
				UPDATE grids
				SET within_nodes = within_nodes || v_id
				WHERE id = grid_id;
			END IF;
		END LOOP;
	END LOOP;


END
$BODY$
LANGUAGE plpgsql VOLATILE;


/*
Populates the vertex table with grid id to which
the vertex belongs
*/
CREATE OR REPLACE FUNCTION ST_PopulateVertices()
	RETURNS void AS
$BODY$
DECLARE
within BOOLEAN;	
v_id INTEGER;
v_geom geometry;
g_id INTEGER;	
BEGIN	
	FOR v_id, v_geom IN
		SELECT id as v_id, the_geom as v_geom
		FROM cleaned_ways_vertices_pgr
	LOOP
		FOR g_id, within IN
			SELECT id,
			ST_Intersects(v_geom, the_geom)
			FROM grids
		LOOP
			IF within = TRUE THEN
				UPDATE cleaned_ways_vertices_pgr
					SET grid_id = g_id
					WHERE id = v_id;
			END IF;
		END LOOP;
	END LOOP;

END
$BODY$
LANGUAGE plpgsql VOLATILE;

/*
Returns the shortest path by including only those edges
in the surrounding grids to which the source and target
belong to.
*/
CREATE OR REPLACE FUNCTION grid_buffer_dijkstra(
	start_vid BIGINT,
	end_vid BIGINT,
	directed BOOLEAN DEFAULT true,
	buffer INTEGER DEFAULT 0,
  	OUT seq INTEGER,
  	OUT node BIGINT,
  	OUT edge BIGINT,
  	OUT g_id INTEGER,
  	OUT cost float,
  	OUT agg_cost float)
RETURNS SETOF RECORD AS
$BODY$
DECLARE
gid1 INTEGER;
gid2 INTEGER;
x1 INTEGER;
y1 INTEGER;
x2 INTEGER;
y2 INTEGER;
max_x INTEGER;
min_x INTEGER;
max_y INTEGER;
min_y INTEGER;
n INTEGER;
selected_nodes INTEGER[];
BEGIN
    gid1 := (SELECT grid_id FROM ways_vertices_pgr WHERE id = $1);
    gid2 := (SELECT grid_id FROM ways_vertices_pgr WHERE id = $2);
    x1 := (SELECT x FROM grids WHERE id = gid1);
    y1 := (SELECT y FROM grids WHERE id = gid1);
    x2 := (SELECT x FROM grids WHERE id = gid2); 
    y2 := (SELECT y FROM grids WHERE id = gid2);

    n := (SELECT SQRT(count(*)) FROM grids);

    max_x := (SELECT GREATEST(x1, x2));
    min_x := (SELECT LEAST(x1, x2));
    max_y := (SELECT GREATEST(y1, y2));
    min_y := (SELECT LEAST(y1, y2));
    
    /*
	Setting buffer limits
    */
    min_x:= min_x - buffer;
    min_y:= min_y - buffer;

    max_x:= max_x + buffer;
    max_y:= max_y + buffer;

    IF min_x < 1 THEN
    	min_x:= 1;
    END IF;

    IF min_y < 1 THEN
    	min_y:= 1;
    END IF;

    IF max_x > n THEN
    	max_x:= n;
    END IF;

    IF max_y > n THEN
    	max_y:= n;
    END IF;

    /*
	Selecting grids within buffer limits
    */
    FOR i in min_x..max_x LOOP	
    	FOR j in min_y..max_y LOOP
    		selected_nodes := ARRAY(SELECT DISTINCT UNNEST(selected_nodes || (SELECT within_nodes FROM grids WHERE x = i AND y = j) 
    		));
    	
    	END LOOP;
    END LOOP;

    RETURN QUERY
    SELECT P.seq, P.node, P.edge,
    (SELECT grid_id FROM ways_vertices_pgr WHERE id = P.node) AS g_id,
    P.cost, P.agg_cost FROM
    pgr_dijkstra('SELECT gid AS id, source, target, cost, reverse_cost FROM ways WHERE source = ANY ('
    	|| quote_literal(selected_nodes) || ') AND target = ANY (' || quote_literal(selected_nodes) || ')',
    	start_vid, end_vid, directed) P; 
END
$BODY$
LANGUAGE plpgsql VOLATILE;


/*
Returns the shortest path along with the grid id of every
node in the path
*/
CREATE OR REPLACE FUNCTION grid_dijkstra(
	start_vid BIGINT,
	end_vid BIGINT,
	directed BOOLEAN DEFAULT true,
  	OUT seq integer,
  	OUT node BIGINT,
  	OUT edge BIGINT,
  	OUT g_id INTEGER,
  	OUT cost float,
  	OUT agg_cost float)
RETURNS SETOF RECORD AS
$BODY$
BEGIN

    RETURN QUERY
    SELECT P.seq, P.node, P.edge,
    (SELECT grid_id FROM ways_vertices_pgr WHERE id = P.node) AS g_id,
    P.cost, P.agg_cost FROM
    pgr_dijkstra('SELECT gid AS id, source, target, cost, reverse_cost FROM ways'
    	, start_vid, end_vid, directed) P; 
END
$BODY$
LANGUAGE plpgsql VOLATILE;