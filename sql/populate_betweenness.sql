
DROP TABLE IF EXISTS tmp; -- else it is dropped at end of session automatically
CREATE TEMP TABLE tmp (

   id                BIGINT                  NOT NULL,
   level             INTEGER,
   betweenness       DOUBLE PRECISION        DEFAULT 0.00
);

SET file.name to :db;
--SHOW file.name;

DO $$
DECLARE 
file_path TEXT;
file_name TEXT;
BEGIN
	file_name := current_setting('file.name');
	file_path := format('/home/vrgeo/rohith/research/pgrouting-optimisation/data/%s_betweenness.csv', file_name);
	RAISE NOTICE '%', file_path;    
	EXECUTE format('COPY tmp FROM %s delimiter '||quote_literal(',')||' csv', quote_literal(file_path));
END$$;


CREATE INDEX results_index ON tmp(id);

UPDATE cleaned_ways
SET    level = tmp.level, promoted_level = tmp.level, betweenness = tmp.betweenness 
FROM   tmp
WHERE  cleaned_ways.id = tmp.id;
/*
UPDATE cleaned_ways_vertices_pgr
FROM tmp
WHERE cleaned_ways_vertices_pgr.id = tmp.source;
*/
DROP TABLE tmp; -- else it is dropped at end of session automatically
