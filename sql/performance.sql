
DROP FUNCTION performanceAnalysis(TEXT,TEXT,INTEGER,INTEGER,INTEGER,BOOLEAN); 
CREATE OR REPLACE FUNCTION performanceAnalysis(
    edge_table TEXT,
    vertex_table TEXT,
    num_pairs INTEGER,
    skeleton_level INTEGER,
    num_levels INTEGER DEFAULT 10,
    num_iterations INTEGER DEFAULT 1,
    directed BOOLEAN DEFAULT true,

    
    OUT source BIGINT,
    OUT target BIGINT,
    OUT level INTEGER,
    OUT num_edges BIGINT,
    OUT num_vertices BIGINT,
    OUT graph_build_time FLOAT,
    OUT avg_computation_time FLOAT,
    OUT path_len FLOAT)
RETURNS SETOF RECORD AS
$body$
DECLARE
  source_comp INTEGER;
  target_comp INTEGER;
  source_sql TEXT;
  target_sql TEXT;
  final_sql TEXT;
  start_vids BIGINT[];
  end_vids BIGINT[];
BEGIN
    /*
    start_vids := ARRAY(
    SELECT id
    FROM vertex_table
    ORDER BY RANDOM() 
    LIMIT num_pairs
    );
    end_vids := ARRAY(
    SELECT id
    FROM vertex_table
    ORDER BY RANDOM() 
    LIMIT num_pairs
    );
    */
    EXECUTE 'SELECT ARRAY(SELECT id FROM '|| vertex_table ||' ORDER BY RANDOM() LIMIT '||num_pairs||')' INTO start_vids;
    EXECUTE 'SELECT ARRAY(SELECT id FROM '|| vertex_table ||' ORDER BY RANDOM() LIMIT '||num_pairs||')' INTO end_vids;
    IF level = num_levels THEN
        final_sql = 'SELECT id, source, target, cost FROM ' || edge_table;
        RETURN QUERY SELECT a.source, a.target, skeleton_level, a.num_edges, a.num_vertices,
            a.graph_build_time, a.avg_computation_time, a.path_len 
            FROM pgr_performanceAnalysis(final_sql, 'pgr_dijkstra', start_vids, end_vids, num_iterations, directed) AS a;
    ELSE 
        FOR i in 1 .. array_upper(start_vids, 1)
        LOOP
            source := start_vids[i];
            target := end_vids[i];
            RAISE NOTICE 'source %', source;
            RAISE NOTICE 'target %', target;
            source_sql := 'SELECT component['|| skeleton_level ||'] FROM ' 
            || vertex_table 
            || ' WHERE id = ' || source;
            target_sql := 'SELECT component['|| skeleton_level ||'] FROM ' 
            || vertex_table 
            || ' WHERE id = ' || target;
            EXECUTE source_sql INTO source_comp;
            EXECUTE target_sql INTO target_comp;

            final_sql := 'SELECT id, source, target, cost FROM '
            || edge_table || ' WHERE promoted_level['|| skeleton_level ||'] = 0 OR component['|| skeleton_level ||'] = ' 
            || source_comp || ' OR component['|| skeleton_level ||'] = ' || target_comp;
            RETURN QUERY SELECT a.source, a.target, skeleton_level, a.num_edges, a.num_vertices,
            a.graph_build_time, a.avg_computation_time, a.path_len 
            FROM pgr_performanceAnalysis(final_sql, 'pgr_dijkstra'::TEXT, ARRAY[source], ARRAY[target], num_iterations, directed) AS a;
        END LOOP;
    END IF;
END
$body$ language plpgsql volatile;