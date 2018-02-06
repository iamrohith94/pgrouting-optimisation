CREATE OR REPLACE FUNCTION pgr_strongly_connect_level(
	edge_table TEXT,
	vertex_table TEXT,
	level TEXT
)
RETURNS VOID AS
$BODY$

DECLARE
skeleton_sql TEXT;
scc_sql TEXT;
level INTEGER;
curr_comps INTEGER[];
curr_geoms GEOMETRY[]; 
BEGIN
	DELETE FROM temp_comp;
	skeleton_sql := 'SELECT id, source, target, cost FROM ' 
		|| edge_table || ' WHERE promoted_level <= ' || level;
	scc_sql := 'SELECT component, array_agg(node) AS nodes FROM pgr_strongComponents(' ||
	quote_literal(skeleton_sql) || ')';
	FOR temp_row IN EXECUTE scc_sql
	LOOP
		SELECT array_length(temp_row.nodes, 1) INTO c_size;
		EXECUTE 'INSERT INTO temp_comp
		SELECT '|| temp_row.component || ', ' || c_size ||', ST_Union(the_geom) 
		FROM '|| vertex_table ||
		' WHERE id = ANY(' || quote_literal(temp_row.vids) || ')';
	END LOOP;
	curr_comps := ARRAY(
	SELECT component 
	FROM temp_comp
	ORDER BY num_nodes DESC
	LIMIT 2
	);
	curr_geoms := ARRAY(
	SELECT comp_geom 
	FROM temp_comp
	ORDER BY num_nodes DESC
	LIMIT 2
	);
	
END;
$BODY$
$BODY$

CREATE TABLE temp_comp(
	comp_id BIGINT,
	num_nodes INTEGER
);

SELECT AddGeometryColumn('temp_comp', 'comp_geom', 0, 'MULTIPOINT', 2 );