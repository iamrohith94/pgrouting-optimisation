CREATE OR REPLACE FUNCTION populate_level(
	edge_table TEXT,
	level INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
skeleton_sql TEXT;
edges_sql TEXT;
conn_sql TEXT;
temp TEXT;
temp_row RECORD;

BEGIN
	RAISE NOTICE 'level %', level;
	skeleton_sql := 'SELECT id, source, target, cost FROM ' 
			|| edge_table || ' WHERE promoted_level <= ' || level;

	edges_sql := 'SELECT id, source, target, cost FROM ' || edge_table;

	FOR temp_row IN SELECT source, array_agg(target)::bigint[] as targets FROM pgr_makeConnected(skeleton_sql) GROUP BY source
	LOOP
		temp = 'ARRAY' || regexp_replace(regexp_replace(temp_row.targets::TEXT, '{', '['), '}', ']');

        EXECUTE 'UPDATE ' || edge_table || ' SET promoted_level = ' || level || ' 
        WHERE id IN 
        (SELECT DISTINCT(edge) 
        	FROM pgr_dijkstra('|| quote_literal(edges_sql) || ', '
        	|| temp_row.source || ', ' || temp || ') WHERE edge != -1) AND promoted_level > ' || level;
    END LOOP;
END;


$BODY$
LANGUAGE plpgsql VOLATILE STRICT;



CREATE OR REPLACE FUNCTION populate_levels(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
level INTEGER;

BEGIN
	FOR level IN 1..num_levels
	LOOP 
		PERFORM populate_level(edge_table, level-1);
	END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


CREATE OR REPLACE FUNCTION compute_connections(
	edge_table TEXT,
	num_levels INTEGER)
RETURNS VOID AS
$BODY$

DECLARE
level INTEGER;

BEGIN
	FOR level IN 1..num_levels
	LOOP 
		SELECT source, array_agg(target)::bigint[] as targets FROM pgr_makeConnected(skeleton_sql) GROUP BY source;
	END LOOP;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;


SELECT populate_levels('cleaned_ways', 10);




