CREATE OR REPLACE FUNCTION pgr_skeletal_path(
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
skeleton_edges_sql TEXT;
contraction_parent_sql TEXT;
skeletal_parent_sql TEXT;
source_contraction_parent BIGINT;
target_contraction_parent BIGINT;
source_skeletal_parent BIGINT;
target_skeletal_parent BIGINT;
BEGIN
	-- Fetching the parent if it is a dead end contracted node
	contraction_parent_sql := 'SELECT parent FROM %s WHERE id = %s';
	EXECUTE format(contraction_parent_sql, vertex_table, source) INTO source_contraction_parent;
	EXECUTE format(contraction_parent_sql, vertex_table, target) INTO target_contraction_parent;

	-- Fetching the skeletal parent
	skeletal_parent_sql := 'SELECT skeletal_parent_%s FROM %s WHERE id = %s'; 
	EXECUTE format(skeletal_parent_sql, level, vertex_table, source_contraction_parent) INTO source_skeletal_parent;
	EXECUTE format(skeletal_parent_sql, level, vertex_table, target_contraction_parent) INTO target_skeletal_parent;

	-- Path Computation
	skeleton_edges_sql := 'SELECT id, source, target, cost FROM %s WHERE ABS(component_%s) = 1';
	RETURN QUERY 
	-- Path on Skeleton
	SELECT foo.path_seq, foo.node, foo.edge, foo.cost, foo.agg_cost FROM pgr_dijkstra(
		format(skeleton_edges_sql, edge_table, level), source_skeletal_parent, target_skeletal_parent) AS foo;
	-- Path from source to skeletal parent of source
	-- Path from skeletal parent of target to target

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;