DROP TABLE IF EXISTS cleaned_ways;
DROP TABLE IF EXISTS cleaned_ways_vertices_pgr;
DROP TABLE IF EXISTS components;
DROP INDEX IF EXISTS st_index;
DROP INDEX IF EXISTS v_index;
CREATE INDEX st_index ON ways(source, target);
CREATE INDEX v_index ON ways_vertices_pgr(id);
/* Cleaned ways table */
CREATE TABLE cleaned_ways(
   id 					BIGINT 		            NOT NULL,
   source           	BIGINT    					NOT NULL,
   target           	BIGINT     					NOT NULL,
   cost        		DOUBLE PRECISION,
   x1                DOUBLE PRECISION,
   y1                DOUBLE PRECISION,
   x2                DOUBLE PRECISION,
   y2                DOUBLE PRECISION,
   betweenness      	DOUBLE PRECISION,
   level  				INTEGER 	DEFAULT 10,
   promoted_level  	INTEGER 	DEFAULT 10,
   component 			INTEGER[] 	DEFAULT '{}'
);
/* Cleaned vertices table */
CREATE TABLE cleaned_ways_vertices_pgr(
   id 				BIGINT 		PRIMARY KEY     NOT NULL,
   component 		INTEGER[] 	DEFAULT '{}'
);
/* Components table */
CREATE TABLE components(
	id 					BIGINT 	 NOT NULL,
	level 				INTEGER  NOT NULL,
	num_nodes			INTEGER,
	num_edges			INTEGER,
	num_border_nodes	INTEGER
);

/* Components table */
CREATE TABLE performance_analysis(
   source               BIGINT   NOT NULL,
   target               BIGINT  NOT NULL,
   level                INTEGER,
   num_edges            INTEGER,
   num_vertices         INTEGER,
   graph_build_time     DOUBLE PRECISION,
   avg_computation_time DOUBLE PRECISION,
   path_len             DOUBLE PRECISION
);


SELECT AddGeometryColumn('cleaned_ways', 'the_geom', 0, 'LINESTRING', 2 );
SELECT AddGeometryColumn('cleaned_ways_vertices_pgr', 'the_geom', 0, 'POINT', 2 );

/* Generating the cleaned vertex table by selecting edges of the largest component */
INSERT INTO cleaned_ways_vertices_pgr(id) 
SELECT unnest(array_agg(node)) 
FROM pgr_strongComponents('select gid as id, source, target, cost, reverse_cost from ways') 
WHERE component = (SELECT component FROM pgr_strongComponents('select gid as id, source, target, cost, reverse_cost from ways') 
GROUP BY component ORDER BY count(*) DESC LIMIT 1) GROUP BY component;

/* Creating index on vertex table */
CREATE INDEX cv_index ON cleaned_ways_vertices_pgr(id);

/*Updating the geometry column of vertices table*/
UPDATE cleaned_ways_vertices_pgr SET the_geom = ways_vertices_pgr.the_geom 
FROM ways_vertices_pgr
WHERE cleaned_ways_vertices_pgr.id = ways_vertices_pgr.id;

/* Generating the cleaned edge table */
INSERT INTO cleaned_ways(id, source, target, cost, x1, y1, x2, y2, the_geom)
SELECT gid as id, source, target, cost, x1, y1, x2, y2, the_geom FROM ways WHERE cost > 0 
AND source IN (SELECT id FROM cleaned_ways_vertices_pgr) AND target IN 
(SELECT id FROM cleaned_ways_vertices_pgr)
UNION ALL 
SELECT gid as id, target, source, reverse_cost, x1, y1, x2, y2, the_geom FROM ways WHERE reverse_cost > 0
AND source IN (SELECT id FROM cleaned_ways_vertices_pgr) AND target IN 
(SELECT id FROM cleaned_ways_vertices_pgr);

/* Creating index on edge table */
CREATE INDEX cst_index ON cleaned_ways(source, target);

/* Importing results into temp table */
DO $$
DECLARE file_path TEXT;
DECLARE file_name TEXT;
DECLARE query TEXT;
BEGIN
   /*EXECUTE 'SELECT' 
   file_path := '/home/rohithreddy/mystuff/research/parallel-betweenness/out_'
                  || (SELECT :fname) || '.csv';
   RAISE NOTICE 'File: %', file_path;*/


END
$$;

