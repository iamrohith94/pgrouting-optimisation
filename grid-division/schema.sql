--ALTER TABLE ways DROP COLUMN grids;
--ALTER TABLE ways DROP COLUMN is_within;
--ALTER TABLE ways ADD COLUMN grids INTEGER[] DEFAULT ARRAY[]::INTEGER[];
--ALTER TABLE ways ADD COLUMN is_within BOOLEAN;

DROP TABLE IF EXISTS grids;

CREATE TABLE grids(
	id SERIAL PRIMARY KEY,
	x INTEGER NOT NULL,
	y INTEGER NOT NULL,
	chosen_nodes BIGINT[]
	--within_edges INTEGER[] DEFAULT ARRAY[]::INTEGER[],
	--intersect_edges INTEGER[] DEFAULT ARRAY[]::INTEGER[],
	--within_nodes INTEGER[] DEFAULT ARRAY[]::INTEGER[],
	--area float,
	--density float
);

SELECT AddGeometryColumn('grids', 'the_geom', 4326, 'POLYGON', 2 );

DROP TABLE IF EXISTS bounding_boxes;
CREATE TABLE bounding_boxes(
	id INT NOT NULL,
	the_geom geometry
);

ALTER TABLE cleaned_ways_vertices_pgr ADD COLUMN grid_id INTEGER DEFAULT 0;

/*DROP TABLE IF EXISTS path_differences;
CREATE TABLE path_differences(
	id SERIAL PRIMARY KEY,
	source INT NOT NULL,
	target INT NOT NULL
); 

SELECT AddGeometryColumn('path_differences', 'actual_path', 4326, 'LINESTRING', 2 );
SELECT AddGeometryColumn('path_differences', 'grid_path', 4326, 'LINESTRING', 2 );
*/
