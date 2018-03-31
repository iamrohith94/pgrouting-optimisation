CREATE OR REPLACE FUNCTION grid_betweenness_pairs(
	vertex_table TEXT,
	p INTEGER)
RETURNS BIGINT[] AS

$BODY$
DECLARE
vertices_sql TEXT;
grid_sql TEXT;
expo DOUBLE PRECISION;
g_id INTEGER;
v_count INTEGER;
per_grid_count INTEGER;
num_grids INTEGER;
target_comp INTEGER;
temp_grid_vids BIGINT[];
total_grid_vids BIGINT[];
grid_info RECORD;
BEGIN
	grid_sql := 'SELECT grid_id, count(*) AS vertex_count FROM %s WHERE parent = id GROUP BY grid_id'; 
	vertices_sql := 'SELECT ARRAY(SELECT id FROM %s WHERE grid_id = %s AND parent = id ORDER BY RANDOM() LIMIT %s)';
	expo := 1::decimal / p;
	-- RAISE NOTICE 'exponent %', expo;
	FOR grid_info IN EXECUTE format(grid_sql, vertex_table)
	LOOP
		g_id := grid_info.grid_id;
		v_count := grid_info.vertex_count;
		RAISE NOTICE 'grid id %', g_id;
		RAISE NOTICE 'vertex count %', v_count;
		--EXECUTE 'SELECT POWER(v_count, expo) INTO per_grid_count;
		per_grid_count := POWER(v_count, expo);
		RAISE NOTICE 'per grid count %', per_grid_count;
		EXECUTE format(vertices_sql, vertex_table, g_id, per_grid_count) INTO temp_grid_vids;
		-- RAISE NOTICE 'temp grid vids %', temp_grid_vids;
		total_grid_vids = temp_grid_vids || total_grid_vids; 
	END LOOP;
	-- RAISE NOTICE 'all grid vids %', total_grid_vids;
	-- vertices := total_grid_vids;
	RETURN total_grid_vids;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

DROP TYPE IF EXISTS grid_chosen_vertices CASCADE;

CREATE TYPE grid_chosen_vertices as 
(id INTEGER,
random_vertices BIGINT[] );

CREATE OR REPLACE FUNCTION grid_wise_vertices(
	vertex_table TEXT,
	p INTEGER)
RETURNS SETOF grid_chosen_vertices AS 

$BODY$
DECLARE
vertices_sql TEXT;
grid_sql TEXT;
expo DOUBLE PRECISION;
g_id INTEGER;
v_count INTEGER;
per_grid_count INTEGER;
num_grids INTEGER;
temp grid_chosen_vertices;
grid_info RECORD;
BEGIN
	grid_sql := 'SELECT grid_id, count(*) AS vertex_count FROM %s GROUP BY grid_id'; 
	vertices_sql := 'SELECT grid_id as id, array_agg(id) as random_nodes FROM (SELECT grid_id, id FROM %s WHERE grid_id = %s ORDER BY RANDOM() LIMIT %s) AS foo GROUP BY grid_id';
	expo := 1::decimal / p;
	-- RAISE NOTICE 'exponent %', expo;
	FOR grid_info IN EXECUTE format(grid_sql, vertex_table)
	LOOP
		g_id := grid_info.grid_id;
		v_count := grid_info.vertex_count;
		RAISE NOTICE 'grid id %', g_id;
		RAISE NOTICE 'vertex count %', v_count;
		--EXECUTE 'SELECT POWER(v_count, expo) INTO per_grid_count;
		per_grid_count := POWER(v_count, expo);
		RAISE NOTICE 'per grid count %', per_grid_count;
		EXECUTE format(vertices_sql, vertex_table, g_id, per_grid_count) INTO temp;
		RETURN NEXT temp;
	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


UPDATE grids SET chosen_nodes = foo.random_vertices 
FROM (SELECT id, random_vertices FROM grid_wise_vertices('cleaned_ways_vertices_pgr', 2)) AS foo 
WHERE foo.id = grids.id;