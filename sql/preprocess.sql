DROP TABLE IF EXISTS cleaned_ways;
DROP TABLE IF EXISTS cleaned_ways_vertices_pgr;
DROP TABLE IF EXISTS contracted_ways;
DROP TABLE IF EXISTS contracted_vertices;
DROP TABLE IF EXISTS components;
DROP TABLE IF EXISTS performance_analysis;
DROP INDEX IF EXISTS st_index;
DROP INDEX IF EXISTS v_index;
CREATE INDEX st_index ON ways(source, target);
CREATE INDEX v_index ON ways_vertices_pgr(id);
DROP FUNCTION IF EXISTS create_graph_tables(TEXT, TEXT, INTEGER);
CREATE OR REPLACE FUNCTION create_graph_tables(
   edge_table TEXT,
   vertex_table TEXT,
   num_levels INTEGER)
RETURNS VOID AS
$BODY$
DECLARE
temp_sql TEXT;
ways_sql TEXT;
vertices_sql TEXT;
BEGIN
   
   temp_sql = '';
   FOR level IN 1..num_levels
   LOOP 
      -- RAISE NOTICE 'level %', level;
      IF level = num_levels THEN
         temp_sql := temp_sql || 'component_' || level || ' INTEGER DEFAULT 1';
      ELSE 
         temp_sql := temp_sql || 'component_' || level || ' INTEGER DEFAULT 1, ';
      END IF;
   END LOOP;

   -- RAISE NOTICE 'temp_sql %', temp_sql;

   ways_sql := 'CREATE TABLE ' ||edge_table||'(
   id                BIGINT                  NOT NULL,
   source            BIGINT                  NOT NULL,
   target            BIGINT                  NOT NULL,
   cost              DOUBLE PRECISION,
   x1                DOUBLE PRECISION,
   y1                DOUBLE PRECISION,
   x2                DOUBLE PRECISION,
   y2                DOUBLE PRECISION,
   betweenness       DOUBLE PRECISION,
   level             INTEGER  DEFAULT 10,
   promoted_level    INTEGER  DEFAULT 10, ' || temp_sql || ' );';

   vertices_sql := 'CREATE TABLE ' ||vertex_table||'(
   id                BIGINT                  NOT NULL,
   parent            BIGINT                  ,
    ' || temp_sql || ' );';
   EXECUTE ways_sql;
   EXECUTE vertices_sql;
END;

$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

SELECT create_graph_tables('cleaned_ways', 'cleaned_ways_vertices_pgr', :num_levels);

/* Components table */
CREATE TABLE components(
	id 					BIGINT 	 NOT NULL,
	level 				INTEGER  NOT NULL,
	num_nodes			INTEGER,
	num_edges			INTEGER,
	num_border_nodes	INTEGER
);

/* Performance table */
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
--SELECT id FROM ways_vertices_pgr; 
SELECT unnest(array_agg(node)) 
FROM pgr_strongComponents('select gid as id, source, target, cost, reverse_cost from ways') 
WHERE component = (SELECT component FROM pgr_strongComponents('select gid as id, source, target, cost, reverse_cost from ways') 
GROUP BY component ORDER BY count(*) DESC LIMIT 1) GROUP BY component;

/* Update parent column */
UPDATE cleaned_ways_vertices_pgr SET parent = id;

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
(SELECT id FROM cleaned_ways_vertices_pgr) AND source != target
UNION ALL 
SELECT gid as id, target, source, reverse_cost, x2, y2, x1, y1, the_geom FROM ways WHERE reverse_cost > 0
AND source IN (SELECT id FROM cleaned_ways_vertices_pgr) AND target IN 
(SELECT id FROM cleaned_ways_vertices_pgr) AND source != target;


/*Generating contraction results for vertices*/
SELECT id AS parent, contracted_vertices AS vids INTO contracted_vertices FROM pgr_contractGraph('SELECT id, source, target, cost FROM cleaned_ways', ARRAY[1]);

/*Storing contracted edges*/
SELECT id, source, target INTO contracted_ways FROM cleaned_ways 
WHERE source IN (SELECT distinct(unnest(vids)) FROM contracted_vertices)
OR target IN  (SELECT distinct(unnest(vids)) FROM contracted_vertices);
CREATE INDEX cid_index ON contracted_ways(id);


/*Applying contraction to the cleaned_ways table */
DELETE FROM cleaned_ways 
WHERE id IN (SELECT id FROM contracted_ways);

/* Updating the parent vertices after contraction */
UPDATE cleaned_ways_vertices_pgr SET parent = foo.parent 
FROM (SELECT parent, unnest(vids) AS vid FROM contracted_vertices) AS foo
WHERE cleaned_ways_vertices_pgr.id = foo.vid;
 
/* Creating index on edge table */
CREATE INDEX cst_index ON cleaned_ways(source, target);

--CREATE INDEX cs_index ON cleaned_ways(source);
CREATE INDEX ct_index ON cleaned_ways(target);
CREATE INDEX id_index ON cleaned_ways(id);

/* Creating index on parent column of vertices table */
CREATE INDEX p_index ON cleaned_ways_vertices_pgr(parent);

