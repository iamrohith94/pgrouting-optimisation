
DROP TABLE IF EXISTS tmp; -- else it is dropped at end of session automatically
CREATE TEMP TABLE tmp (

   id                BIGINT                  NOT NULL,
   level             INTEGER,
   betweenness       DOUBLE PRECISION        DEFAULT 0.00
);

COPY tmp FROM '/home/vrgeo/rohith/research/pgrouting-optimisation/data/melbourne_betweenness.csv' delimiter ',' csv;
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
