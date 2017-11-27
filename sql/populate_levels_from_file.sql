DROP TABLE IF EXISTS final_tmp;
CREATE TEMP TABLE tmp (

   id                BIGINT                  NOT NULL,
   source            BIGINT                  NOT NULL,
   target            BIGINT                  NOT NULL,
   level             INTEGER
);

COPY tmp FROM '/home/rohithreddy/mystuff/research/pgrouting-optimisation/data/chandigarh_cars_conn_edges.csv' delimiter ',' csv;

SELECT id, source, target, min(level) AS level INTO final_tmp FROM tmp GROUP BY id, source, target;

CREATE INDEX results_index ON final_tmp(id, source, target);


UPDATE cleaned_ways
SET    promoted_level = final_tmp.level
FROM   final_tmp
WHERE  cleaned_ways.id = final_tmp.id 
AND cleaned_ways.source = final_tmp.source AND cleaned_ways.target = final_tmp.target 
AND cleaned_ways.promoted_level > final_tmp.level;
/*
UPDATE cleaned_ways_vertices_pgr
FROM tmp
WHERE cleaned_ways_vertices_pgr.id = tmp.source;
*/
DROP TABLE tmp; -- else it is dropped at end of session automatically
