SET num.levels to :num_levels;
DO $$
DECLARE
num_levels INTEGER;
BEGIN
	num_levels := current_setting('num.levels');
	FOR level in 1 .. num_levels
	LOOP
		RAISE NOTICE 'Level %', level;
		EXECUTE format('CREATE INDEX IF NOT EXISTS v_comp_idx_%s ON cleaned_ways_vertices_pgr(component_%s)', 
						level, level);
		EXECUTE format('CREATE INDEX IF NOT EXISTS e_comp_idx_%s ON cleaned_ways(component_%s)', 
						level, level); 
	END LOOP;
END$$;