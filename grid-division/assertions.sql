CREATE OR REPLACE FUNCTION assert_unassigned_vertices(
	vertex_table TEXT)
RETURNS VOID AS
$BODY$

DECLARE
unass_count_query TEXT;
count INTEGER;
BEGIN
	unass_count_query := 'SELECT count(*) FROM '
	|| vertex_table || ' WHERE grid_id = 0;';
	EXECUTE unass_count_query INTO count;
	ASSERT count = 0;

END;
$BODY$
LANGUAGE plpgsql VOLATILE STRICT;

SELECT assert_unassigned_vertices('cleaned_ways_vertices_pgr');
