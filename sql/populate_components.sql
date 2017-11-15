CREATE OR REPLACE FUNCTION populate_components(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
level INTEGER;
c_size INTEGER;
skeleton_sql TEXT;
component_sql TEXT;
temp_row RECORD;
BEGIN
	UPDATE cleaned_ways SET component = array_fill(0, ARRAY[num_levels]);
	UPDATE cleaned_ways_vertices_pgr SET component = array_fill(0, ARRAY[num_levels]);
	FOR level IN 1..num_levels
	LOOP 
		RAISE NOTICE 'level %', level;
		skeleton_sql := 'SELECT id, source, target, cost FROM ' 
			|| edge_table || ' WHERE promoted_level[' || level || '] != 0';
		component_sql := 'SELECT component, array_agg(node) AS vids FROM pgr_connectedComponents(' ||
		quote_literal(skeleton_sql) || ') GROUP BY component;';
		
		FOR temp_row IN EXECUTE component_sql
		LOOP
			SELECT array_length(temp_row.vids, 1) INTO c_size;
			EXECUTE 'UPDATE cleaned_ways 
			SET component['|| level || ']=' || temp_row.component ||
			' WHERE (source = ANY(' || quote_literal(temp_row.vids) || ') 
			OR target = ANY(' || quote_literal(temp_row.vids) || ')) 
			AND promoted_level[' || level || '] != 0;';
			EXECUTE 'UPDATE cleaned_ways_vertices_pgr 
			SET component['|| level || ']=' || temp_row.component ||
			' WHERE id = ANY(' || quote_literal(temp_row.vids) || ');';
			EXECUTE 'INSERT INTO components(id, level, num_nodes, num_edges) 
			SELECT '|| temp_row.component || ' AS id,'|| level || ' AS level, ' || 
			c_size  || ' AS num_nodes, count(*) AS num_edges FROM cleaned_ways WHERE component['
			||level||'] = ' || temp_row.component||';';
		END LOOP;
	END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT; 

CREATE OR REPLACE FUNCTION populate_boundary_nodes(
	vertex_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
level INTEGER;
skeleton_sql TEXT;
component_sql TEXT;
temp_row RECORD;
BEGIN
	FOR level IN 1..num_levels
	LOOP 

	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT; 

SELECT populate_components('cleaned_ways', 10);
