psql -U postgres -f grid-division/schema.sql -d $1
psql -U postgres -f grid-division/grid_functions.sql -d $1
python grid-division/grid.py $1 $2
psql -U postgres -f grid-division/assertions.sql -d $1


