CREATE OR REPLACE FUNCTION update_nearest_skeletal_node(
	vertex_table TEXT,
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
component_sql TEXT;
component_vertices_sql TEXT;
skeletal_vertices_sql TEXT;
update_sql TEXT;
level INTEGER;
vertex RECORD;
component RECORD;
component_vertex RECORD;
skeletal_vertices BIGINT[];
BEGIN
	-- Fetches vertices component wise excluding skeleton
	component_sql := 'SELECT component_%s AS id, array_agg(id)::BIGINT[] AS vertices 
	FROM %s 
	WHERE component_%s != 1 GROUP BY component_%s';

	-- Fetch geometry of a vertex based on its component
	component_vertices_sql := 'SELECT id, ST_AsText(the_geom) AS the_geom FROM %s WHERE component_%s = %s AND parent = id';
	
	-- Fetches skeletal vertices of a component
	skeletal_vertices_sql := 'SELECT array_agg(foo.id) FROM 
	(SELECT source AS id FROM %s WHERE -component_%s = %s AND target = ANY(%s::BIGINT[])
	UNION
	SELECT target AS id FROM %s WHERE -component_%s = %s AND source = ANY(%s::BIGINT[])) AS foo';
	
	-- Update the skeletal parents
	update_sql := 'UPDATE %s SET skeletal_parent_%s = 
	(SELECT foo.id FROM %s AS foo WHERE foo.id = ANY(%s::BIGINT[]) ORDER BY ST_Distance(ST_GeomFromText(%s,4326), foo.the_geom) LIMIT 1) 
	WHERE id = %s AND id = parent';

	FOR level IN 1..num_levels
	LOOP 
		RAISE NOTICE 'Level: %',level;
		EXECUTE format('UPDATE %s SET skeletal_parent_%s = id WHERE component_%s = 1', vertex_table, level, level);
		FOR component IN EXECUTE format(component_sql, level, vertex_table, level, level)
		LOOP
			--RAISE NOTICE 'comp id: %', component.id;
			--RAISE NOTICE 'comp vertices: %', component.vertices;
			EXECUTE format(skeletal_vertices_sql, edge_table, level, component.id, quote_literal(component.vertices),
			edge_table, level, component.id, quote_literal(component.vertices)) INTO skeletal_vertices;
			--RAISE NOTICE 'skeletal vertices: %', skeletal_vertices;
			FOR component_vertex IN EXECUTE format(component_vertices_sql, vertex_table, level, component.id)
			LOOP
				EXECUTE format(update_sql, vertex_table, level, vertex_table, quote_literal(skeletal_vertices), quote_literal(component_vertex.the_geom), component_vertex.id);
			END LOOP;
		END LOOP;
	END LOOP;
	
	/*
	read_sql := 'SELECT id, component_%s FROM ST_AsText(the_geom) AS the_geom FROM %s';
	
	FOR level IN 1..num_levels
	LOOP 
		FOR vertex IN EXECUTE format(read_sql, vertex_table)
		LOOP 
			--RAISE NOTICE 'Query: %', format(update_sql, vertex_table, level, vertex_table, level, quote_literal(vertex.the_geom), vertex.id);
			EXECUTE format(update_sql, vertex_table, level, vertex_table, level, quote_literal(vertex.the_geom), vertex.id);
		END LOOP;
	END LOOP;
	*/
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

SELECT update_nearest_skeletal_node('cleaned_ways_vertices_pgr','cleaned_ways', :num_levels);