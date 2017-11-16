DROP INDEX IF EXISTS level_index;
DROP INDEX IF EXISTS plevel_index;
CREATE INDEX level_index ON cleaned_ways(level);
CREATE INDEX plevel_index ON cleaned_ways(promoted_level);