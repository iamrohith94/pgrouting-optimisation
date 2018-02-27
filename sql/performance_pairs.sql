--DROP TABLE IF EXISTS performance_analysis;
DROP FUNCTION IF EXISTS getpairsforperformanceanalysis(text,text,integer,integer);
DROP FUNCTION IF EXISTS getlevelwiseperformanceanalysis(text,text,integer);

-- Table to store performance results level wise
CREATE TABLE IF NOT EXISTS performance_analysis(
	source 					BIGINT 	 NOT NULL,
	target              	BIGINT   NOT NULL,
	level 					INTEGER  NOT NULL,
	qset 					INTEGER NOT NULL,
	num_edges 				BIGINT NOT NULL,
	num_vertices 			BIGINT NOT NULL,
	edge_read_time 		FLOAT NOT NULL,
	graph_build_time 		FLOAT NOT NULL,
	avg_computation_time 	FLOAT NOT NULL,
	path_len 				FLOAT NOT NULL
);

-- Function that fetches the farthest vertex pair
CREATE OR REPLACE FUNCTION getMaxDistancePair(
	vertex_table TEXT,
    OUT source BIGINT,
    OUT target BIGINT)

RETURNS SETOF RECORD AS
$body$
DECLARE
convex_hull TEXT;
convex_hull_points TEXT;
distance_query TEXT;
id_query TEXT;
point_geoms_query TEXT;
point_geoms GEOMETRY[];
source_geom GEOMETRY;
target_geom GEOMETRY;
temp_distance FLOAT;
max_distance FLOAT;
BEGIN
	convex_hull := 'SELECT ST_ConvexHull(ST_Collect(the_geom)) AS the_geom FROM %s WHERE id = parent';
	convex_hull_points := 'SELECT (ST_DumpPoints(g.the_geom)).geom FROM(%s) AS g';
	point_geoms_query := 'SELECT ARRAY(%s)';
	distance_query := 'SELECT ST_Distance(%s, %s)';
	id_query := 'SELECT id FROM %s WHERE the_geom = %s';
	-- RAISE NOTICE 'total_query %', format(convex_hull_points, format(convex_hull, vertex_table));
	EXECUTE format(point_geoms_query, format(convex_hull_points, format(convex_hull, vertex_table))) INTO point_geoms;
	max_distance := -1.00;
	FOR i in 1 .. array_upper(point_geoms, 1)
	LOOP
		FOR j in 1 .. array_upper(point_geoms, 1)
		LOOP
			EXECUTE format(distance_query, quote_literal(point_geoms[i]::text), quote_literal(point_geoms[j]::text)) INTO temp_distance;
			IF temp_distance > max_distance THEN
				max_distance = temp_distance;
				source_geom = point_geoms[i];
				target_geom = point_geoms[j];
			END IF;
		END LOOP;
	END LOOP;
	EXECUTE format(id_query, vertex_table, quote_literal(source_geom::text)) INTO source;
	EXECUTE format(id_query, vertex_table, quote_literal(target_geom::text)) INTO target;
	RAISE NOTICE 'source %', source;
	RAISE NOTICE 'target %', target;
	RETURN QUERY SELECT source, target;
END
$body$ language plpgsql volatile;

-- Checks if all the array values are 0
-- Returns false even if one array value is non zero
CREATE OR REPLACE FUNCTION check_termination(
	count BIGINT[])
RETURNS BOOLEAN AS
$body$
DECLARE
BEGIN
	FOR i in 1 .. array_upper(count, 1)
	LOOP
		IF count[i] != 0 THEN
			RETURN 'f';
		END IF;
	END LOOP;
	RETURN 't';
END
$body$ language plpgsql volatile;

-- A function that fetches source target pairs based on the distance between them
-- It outputs the cost between the source and target on the entire network
CREATE OR REPLACE FUNCTION getPairsForPerformanceAnalysis(
	cleaned_edge_table TEXT, -- Edge table after dead end contraction
	--scc_edge_table TEXT, -- Edge table before dead end contraction
	vertex_table TEXT,
    pairs_per_set INTEGER, -- Number of pairs per set
    num_sets INTEGER, 	   -- Number of sets
    OUT source BIGINT,
    OUT target BIGINT,
    OUT level INTEGER,
    OUT qset INTEGER,
    OUT num_edges BIGINT,
    OUT num_vertices BIGINT,
    OUT edge_read_time FLOAT,
    OUT graph_build_time FLOAT,
    OUT avg_computation_time FLOAT,
    OUT path_len FLOAT
)
RETURNS SETOF RECORD AS
$body$
DECLARE
max_source BIGINT;
max_target BIGINT;
max_distance FLOAT;
curr_distance FLOAT;
count_qset INTEGER[];
distance_query TEXT;
total_query TEXT;
edges_query TEXT;
start_vids BIGINT[];
end_vids BIGINT[];
pairs_iteration INTEGER;
distance_bucket FLOAT;
bucket INTEGER;
temp_record RECORD;
BEGIN
	pairs_iteration := 20;
	edges_query := 'SELECT id, source, target, cost FROM %s';
	distance_query := 'SELECT agg_cost FROM pgr_dijkstra(%s, %s, %s) order by agg_cost desc limit 1;';
	EXECUTE format('SELECT source, target FROM getMaxDistancePair(%s)', quote_literal(vertex_table)) INTO max_source, max_target;
	EXECUTE format(distance_query, quote_literal(format(edges_query, cleaned_edge_table)), max_source, max_target) INTO max_distance;
	RAISE NOTICE 'max_distance %', max_distance;
	RAISE NOTICE 'num_sets %', num_sets;
	RAISE NOTICE 'pairs_per_set %', pairs_per_set;
	distance_bucket = max_distance*1.000/num_sets;

	-- Initialising the counts for each query set
	FOR i in 1 .. num_sets LOOP
		count_qset = count_qset || pairs_per_set;
	END LOOP;

	-- Running the loop until all the query sets are filled
	WHILE NOT check_termination(count_qset) LOOP
		EXECUTE 'SELECT ARRAY(SELECT id FROM '|| vertex_table ||'  WHERE id = parent ORDER BY RANDOM() LIMIT '||pairs_iteration||')' INTO start_vids;
    	EXECUTE 'SELECT ARRAY(SELECT id FROM '|| vertex_table ||'  WHERE id = parent ORDER BY RANDOM() LIMIT '||pairs_iteration||')' INTO end_vids;
    	-- run performance analysis on source, target pairs and get the path_len
    	FOR temp_record IN SELECT * FROM pgr_performanceAnalysis(format(edges_query, cleaned_edge_table), 'pgr_dijkstra', start_vids, end_vids) LOOP
    		bucket = CEIL(temp_record.path_len/distance_bucket);
			--RAISE NOTICE 'bucket %', bucket;
    		IF bucket < num_sets+1 THEN
    			IF count_qset[bucket] > 0 THEN
    				count_qset[bucket] := count_qset[bucket] - 1;
    				RETURN QUERY SELECT temp_record.source, temp_record.target, 10, bucket, temp_record.num_edges, 
    				temp_record.num_vertices, temp_record.edges_read_time, temp_record.graph_build_time, 
    				temp_record.avg_computation_time, temp_record.path_len;
    			END IF;
    		END IF;
    	END LOOP;
    	-- store performance analysis results if path_len falls into the buckets
    	-- return the performance analysis results along with the query set id
	END LOOP;

END
$body$ language plpgsql volatile;


/*
SELECT * INTO scc_ways FROM 
(SELECT gid as id, source, target, cost, x1, y1, x2, y2, the_geom FROM ways WHERE cost > 0 
AND source IN (SELECT id FROM cleaned_ways_vertices_pgr) AND target IN 
(SELECT id FROM cleaned_ways_vertices_pgr) AND source != target
UNION ALL 
SELECT gid as id, target, source, reverse_cost, x2, y2, x1, y1, the_geom FROM ways WHERE reverse_cost > 0
AND source IN (SELECT id FROM cleaned_ways_vertices_pgr) AND target IN 
(SELECT id FROM cleaned_ways_vertices_pgr) AND source != target) AS foo;
*/

CREATE OR REPLACE FUNCTION getLevelWisePerformanceAnalysis(
	edge_table TEXT,
	--scc_table TEXT,
	vertex_table TEXT,
	max_level INTEGER,
    OUT source BIGINT,
    OUT target BIGINT,
    OUT level INTEGER,
    OUT qset INTEGER,
    OUT num_edges BIGINT,
    OUT num_vertices BIGINT,
    OUT edge_read_time FLOAT,
    OUT graph_build_time FLOAT,
    OUT avg_computation_time FLOAT,
    OUT path_len FLOAT
) RETURNS SETOF RECORD AS
$body$
DECLARE
temp_record RECORD;
level INTEGER;
zone_sql TEXT;
final_sql TEXT;
source_comp INTEGER;
target_comp INTEGER;
count INTEGER;
BEGIN
	RAISE NOTICE 'Started computing for levels';	
	zone_sql := 'SELECT ABS(component_%s) FROM %s WHERE id = %s';
	count := 0;
	
	FOR level in 1 .. max_level 
	LOOP
		RAISE NOTICE 'level %', level;
		FOR temp_record IN SELECT foo.source, foo.target, foo.qset 
		FROM (SELECT pa.source AS source, pa.target AS target, pa.qset AS qset, count(*) AS count 
		FROM performance_analysis AS pa 
		GROUP BY pa.source, pa.target, pa.qset) AS foo 
		WHERE foo.count = 1
		LOOP
			EXECUTE format(zone_sql, level, vertex_table, temp_record.source) INTO source_comp;
			EXECUTE format(zone_sql, level, vertex_table, temp_record.target) INTO target_comp;

			--RAISE NOTICE 'source %', temp_record.source;
			--RAISE NOTICE 'target %', temp_record.target;

			--RAISE NOTICE 'source_comp %', source_comp;
			--RAISE NOTICE 'target_comp %', target_comp;

			final_sql := 'SELECT id, source, target, cost FROM '
			|| edge_table || ' WHERE ABS(component_'|| level ||') = 1 OR ABS(component_'|| level ||') = ' 
			|| source_comp || ' OR ABS(component_'|| level ||') = ' || target_comp;

			RETURN QUERY 
			SELECT a.source, a.target, level, temp_record.qset, a.num_edges, a.num_vertices,
			a.edges_read_time, a.graph_build_time, a.avg_computation_time, a.path_len 
			FROM (SELECT * 
			FROM pgr_performanceAnalysis(final_sql, 'pgr_dijkstra'::TEXT, ARRAY[temp_record.source]::BIGINT[], ARRAY[temp_record.target]::BIGINT[])) AS a;
		END LOOP;
		-- RAISE NOTICE 'count %', count;
		-- count := count + 1;
	END LOOP;
END
$body$ language plpgsql volatile;

-- Storing the vertex pairs based on distance
INSERT INTO performance_analysis 
SELECT * FROM getpairsforperformanceanalysis('cleaned_ways', 'cleaned_ways_vertices_pgr', 300, 5);

-- Storing the path cost between the vertex pair for all levels
INSERT INTO performance_analysis 
SELECT * FROM getLevelWisePerformanceAnalysis('cleaned_ways', 'cleaned_ways_vertices_pgr', 8);
