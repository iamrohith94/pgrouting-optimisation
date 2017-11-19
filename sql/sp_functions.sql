CREATE OR REPLACE FUNCTION pgr_approx_path(
	edge_table TEXT,
	vertex_table TEXT,
	source BIGINT,
	target BIGINT,
	level INTEGER,
	OUT seq INTEGER,
	OUT node BIGINT,
	OUT edge BIGINT,
	OUT cost DOUBLE PRECISION,
	OUT agg_cost DOUBLE PRECISION)
RETURNS SETOF RECORD AS

$BODY$
DECLARE
edges_sql TEXT;
comp_sql TEXT;
source_comp INTEGER;
target_comp INTEGER;
BEGIN
	comp_sql := 'SELECT component_%s FROM %s WHERE id = %s'; 
	EXECUTE format(comp_sql, level, edge_table, source) INTO source_comp;
	EXECUTE format(comp_sql, level, edge_table, target) INTO target_comp;
	edges_sql := 'SELECT id, source, target, cost FROM %s WHERE component_%s = 1 OR component_%s = %s OR component_%s = %s';
	RETURN QUERY 
	SELECT foo.path_seq, foo.node, foo.edge, foo.cost, foo.agg_cost FROM pgr_dijkstra(
		format(edges_sql, edge_table, level, 
			level, level, source_comp,
			level, target_comp), source, target) AS foo;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;