CREATE OR REPLACE FUNCTION grid_betweenness_pairs(
	vertex_table TEXT,
	-- k INTEGER,
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