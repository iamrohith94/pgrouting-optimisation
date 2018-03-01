CREATE OR REPLACE FUNCTION assert_skeleton_strongly_connected(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
skeleton_sql TEXT;
comp_count_query TEXT;
level INTEGER;
comp_count INTEGER;
comp_id INTEGER;
BEGIN
	FOR level IN 1..num_levels
	LOOP 
		skeleton_sql := 'SELECT id, source, target, cost FROM ' 
			|| edge_table || ' WHERE component_' || level || ' = 1;';
		comp_count_query := 'SELECT count(DISTINCT(component)) FROM pgr_strongComponents('
		|| quote_literal(skeleton_sql) || ');';
		EXECUTE comp_count_query INTO comp_count;
		ASSERT comp_count = 1;
	END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


CREATE OR REPLACE FUNCTION assert_unassigned_component(
        edge_table TEXT,
	vertex_table TEXT,
        num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
edge_sql_count TEXT;
vertex_sql_count TEXT;
edge_count INTEGER;
level INTEGER;
vertex_count INTEGER;
BEGIN
        FOR level IN 1..num_levels
        LOOP
                edge_sql_count := 'SELECT count(*) FROM '
                        || edge_table || ' WHERE component_' || level || ' = 0' ;
                EXECUTE edge_sql_count INTO edge_count;
                ASSERT edge_count = 0;
		 vertex_sql_count := 'SELECT count(*) FROM '
                        || vertex_table || ' WHERE component_' || level || ' = 0 AND parent = id' ;
                EXECUTE vertex_sql_count INTO vertex_count;
                ASSERT vertex_count = 0;
        END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


/*
CREATE OR REPLACE FUNCTION assert_skeleton_comp_id(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
skeleton_sql TEXT;
level INTEGER;
comp_count INTEGER;
BEGIN
	FOR level IN 1..num_levels
	LOOP 
		skeleton_sql := 'SELECT count(*) FROM ' 
			|| edge_table || ' WHERE promoted_level <= ' || level
			|| ' AND component_' || level || ' != 1';
		EXECUTE skeleton_sql INTO comp_count;
		ASSERT comp_count = 0;
	END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;
*/

CREATE OR REPLACE FUNCTION assert_edge_count(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
edges_sql TEXT;
count_sql TEXT;
level INTEGER;
total_edges INTEGER;
level_edges INTEGER;
BEGIN
	edges_sql := 'SELECT count(*) FROM ' || edge_table;
	EXECUTE edges_sql INTO total_edges;
	FOR level IN 1..num_levels
	LOOP 
		count_sql := 'SELECT SUM(foo.counts) FROM (SELECT count(*) as counts from '
		|| edge_table ||' GROUP BY ABS(component_'||level||')) as foo;';
		EXECUTE count_sql INTO level_edges;
		ASSERT level_edges = total_edges;
	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

CREATE OR REPLACE FUNCTION assert_vertex_count(
	vertex_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
vertices_sql TEXT;
count_sql TEXT;
level INTEGER;
total_vertices INTEGER;
level_vertices INTEGER;
BEGIN
	vertices_sql := 'SELECT count(*) FROM ' || vertex_table;
	EXECUTE vertices_sql INTO total_vertices;
	FOR level IN 1..num_levels
	LOOP 
		count_sql := 'SELECT SUM(foo.counts) FROM (SELECT count(*) as counts from '
		|| vertex_table ||' GROUP BY ABS(component_'||level||')) as foo;';
		EXECUTE count_sql INTO level_vertices;
		ASSERT level_vertices = total_vertices;
	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

CREATE OR REPLACE FUNCTION assert_skeletal_parent(
	vertex_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$
DECLARE
vertices_sql TEXT;
level INTEGER;
total_vertices INTEGER;
BEGIN
	vertices_sql := 'SELECT count(*) FROM %s WHERE skeletal_parent_%s = -1 AND id = parent';
	FOR level IN 1..num_levels
	LOOP 
		--RAISE NOTICE '%', level;
		EXECUTE format(vertices_sql, vertex_table, level) INTO total_vertices;
		ASSERT total_vertices = 0;
	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


CREATE OR REPLACE FUNCTION assert_component_connectivity(
	edge_table TEXT,
	vertex_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$
DECLARE
component RECORD;
component_sql TEXT;
component_vertices_sql TEXT;
component_edges_sql TEXT;
comp_count_sql TEXT;
level INTEGER;
component_vertices BIGINT[];
comp_count INTEGER;
BEGIN
	-- Fetches All vertices of extended component(component + skeletal nodes)
	component_vertices_sql := 'SELECT source AS id FROM %s WHERE ABS(component_%s) = %s 
	UNION 
	SELECT target AS id FROM %s WHERE ABS(component_%s) = %s';

	-- Fetches component ids at a level
	component_sql := 'SELECT comp_id FROM (SELECT ABS(component_%s) AS comp_id, array_length(array_agg(id), 1) AS comp_length FROM %s 
		WHERE ABS(component_%s) != 1  GROUP BY ABS(component_%s)) AS foo WHERE foo.comp_length > 2';

	-- Fetches all edges of a component
	component_edges_sql := 'SELECT id, source, target, cost FROM %s WHERE component_%s = %s';

	-- Fetches the number of strongly connected components of the given component
	comp_count_sql := 'SELECT count(DISTINCT(component)) FROM pgr_strongComponents(%s);';

	FOR level IN 1..num_levels
	LOOP 
		RAISE NOTICE '%', level;
		FOR component IN EXECUTE format(component_sql, level, vertex_table, level, level)
		LOOP
			RAISE NOTICE 'Component %', component.comp_id;
			EXECUTE format(comp_count_sql, 
				quote_literal(format(component_edges_sql,
					edge_table, level, component.comp_id))) INTO comp_count;
			ASSERT comp_count = 1 OR comp_count = 0;
		END LOOP;
	END LOOP;
END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

SELECT assert_skeleton_strongly_connected('cleaned_ways', :num_levels);
SELECT assert_unassigned_component('cleaned_ways', 'cleaned_ways_vertices_pgr', :num_levels);
--SELECT assert_skeleton_comp_id('cleaned_ways', :num_levels);
SELECT assert_edge_count('cleaned_ways', :num_levels);
SELECT assert_vertex_count('cleaned_ways_vertices_pgr', :num_levels);
--SELECT assert_skeletal_parent('cleaned_ways_vertices_pgr', :num_levels);
--SELECT assert_component_connectivity('cleaned_ways', 'cleaned_ways_vertices_pgr', :num_levels);