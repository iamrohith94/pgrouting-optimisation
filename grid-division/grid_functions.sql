


/*
Populates the vertex table with grid id to which
the vertex belongs
*/
CREATE OR REPLACE FUNCTION ST_PopulateVertices()
	RETURNS void AS
$BODY$
DECLARE
within BOOLEAN;	
v_id INTEGER;
v_geom geometry;
g_id INTEGER;	
BEGIN	
	FOR v_id, v_geom IN
		SELECT id as v_id, the_geom as v_geom
		FROM cleaned_ways_vertices_pgr
	LOOP
		FOR g_id, within IN
			SELECT id,
			ST_Intersects(v_geom, the_geom)
			FROM grids
		LOOP
			IF within = TRUE THEN
				UPDATE cleaned_ways_vertices_pgr
					SET grid_id = g_id
					WHERE id = v_id;
			END IF;
		END LOOP;
	END LOOP;

END
$BODY$
LANGUAGE plpgsql VOLATILE;



DROP   TYPE T_Grid cascade;
CREATE TYPE T_Grid AS
(gcol  int4,
 grow  int4,
 geom  geometry);

CREATE OR REPLACE FUNCTION RegularGridXY( p_xmin       NUMERIC,
                                          p_ymin       NUMERIC,
                                          p_xmax       NUMERIC,
                                          p_ymax       NUMERIC,
                                          p_TileSizeX  NUMERIC,
                                          p_TileSizeY  NUMERIC,
                                          num_rows integer,
                                          num_columns integer,
                                          p_srid       integer)
RETURNS SETOF T_Grid IMMUTABLE
AS $$
DECLARE
   v_loCol int4;
   v_hiCol int4;
   v_loRow int4;
   v_hiRow int4;
   v_geom  geometry;
   v_grid  t_grid;
BEGIN
   FOR v_col IN 0..num_columns-1 Loop
     FOR v_row IN 0..num_rows-1 Loop
         v_geom := ST_SetSRID(ST_MakeBox2D(ST_Point(( p_xmin + v_col * p_TileSizeX), (p_ymin + v_row * p_TileSizeY)),
                                           ST_Point((p_xmin+(v_col * p_TileSizeX)+p_TileSizeX), (p_ymin+(v_row * p_TileSizeY)+p_TileSizeY))),
                                           p_srid);
         SELECT v_col,v_row,v_geom INTO v_grid;
         -- SELECT v_col,v_row,ST_GeomFromText('POINT(' || v_col || ' ' || v_row ||')',0) INTO v_grid;
         RETURN NEXT v_grid;
     END Loop;
   END Loop;
END;
$$ LANGUAGE 'plpgsql';