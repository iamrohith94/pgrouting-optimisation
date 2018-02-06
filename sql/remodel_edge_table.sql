SELECT id, 
min(betweenness) AS min_betweenness, 
max(betweenness) AS max_betweenness, 
min(level) AS min_level, 
max(level) AS max_level,
count(*) AS count INTO temp_betweenness
FROM cleaned_ways 
WHERE count > 1
GROUP BY id;

CREATE INDEX bid_index ON temp_betweenness(id);

DELETE FROM cleaned_ways WHERE id in (SELECT id FROM temp_betweenness);

DROP INDEX IF EXISTS id_index;
DROP INDEX IF EXISTS ct_index;
DROP INDEX IF EXISTS cst_index;


INSERT INTO cleaned_ways (id, source, target, cost, reverse_cost)
SELECT id, source, target, cost, reverse_cost 
FROM ways 
WHERE id IN (SELECT id FROM temp_betweenness); 


/* Creating index on edge table */
CREATE INDEX cst_index ON cleaned_ways(source, target);

--CREATE INDEX cs_index ON cleaned_ways(source);
CREATE INDEX ct_index ON cleaned_ways(target);

UPDATE cleaned_ways 
SET betweenness = foo.max_betweenness, level = foo.min_level, promoted_level = foo.min_level 
FROM (SELECT id, max_betweenness, min_level FROM temp_betweenness) AS foo
WHERE foo.id = cleaned_ways.id;


