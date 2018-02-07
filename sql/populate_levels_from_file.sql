DROP TABLE IF EXISTS final_tmp;
CREATE TEMP TABLE tmp (

   id                BIGINT                  NOT NULL,
   source            BIGINT                  NOT NULL,
   target            BIGINT                  NOT NULL,
   level             INTEGER
);

SET file.name to :db;
--SHOW file.name;

DO $$
DECLARE 
file_path TEXT;
file_name TEXT;
BEGIN
	file_name := current_setting('file.name');
	file_path := format('/home/vrgeo/rohith/research/pgrouting-optimisation/data/%s_geom_conn_edges.csv', file_name);
	RAISE NOTICE '%', file_path;    
	EXECUTE format('COPY tmp FROM %s delimiter '||quote_literal(',')||' csv', quote_literal(file_path));
END$$;

--COPY tmp FROM '/home/vrgeo/rohith/research/pgrouting-optimisation/data/chandigarh_foss4g_geom_conn_edges.csv' delimiter ',' csv;

SELECT id, source, target, min(level) AS level INTO final_tmp FROM tmp GROUP BY id, source, target;

CREATE INDEX results_index ON final_tmp(id, source, target);


UPDATE cleaned_ways
SET    promoted_level = final_tmp.level
FROM   final_tmp
WHERE  cleaned_ways.id = final_tmp.id 
AND cleaned_ways.source = final_tmp.source AND cleaned_ways.target = final_tmp.target 
AND cleaned_ways.promoted_level > final_tmp.level;

DROP TABLE tmp; -- else it is dropped at end of session automatically
DROP TABLE final_tmp;

/*
UPDATE cleaned_ways_vertices_pgr
FROM tmp
WHERE cleaned_ways_vertices_pgr.id = tmp.source;
*/
