# $1-> dbname $2-> num_levels

echo 'A. Preprocessing'
echo '1. Network Cleaning'
psql -U postgres $1 -v num_levels=$2 < sql/preprocess.sql
echo '2. Storing Graph in a csv file'
psql -U postgres -d $1 -t -A -F"," -c "select id, source, target, cost from cleaned_ways" > "./data/$1.csv"
echo '3. Network Division into Grids'
bash grid-division/grids.sh $1 4

echo 'Edge Priority and Levels'
echo '1. Creating a function that fetches vertices grid wise'
psql -U postgres $1 < sql/betweenness_st_pairs.sql
echo '2. Computing priority using grid based approach'
time mpirun -n 8 time ./g_b $1 $2 > "data/$1_levels_out.txt"
echo '3. Update priority and level of edges in edge table'
psql -U postgres $1 -v db="'$1'"  < sql/populate_betweenness.sql


echo 'B. Skeleton Generation'
echo '1. Store graph with geometries in a csv file'
psql -U postgres -d $1 -t -A -F"," -c "select id, source, target, cost, x1, y1, x2, y2, level from cleaned_ways" > "./data/$1_geom.csv"
echo '2. Computing connections to make skeleton strongly connected at each level'
time ./c_c "$1_geom" $2
echo '3. Computing paths between the above calculated connections'
time mpirun -n 8 time ./p_c "$1_geom" "$1_geom_conns" > "data/$1_pedges_out.txt"
echo '4. Updating the priority and level of edges in the above paths'
psql -U postgres $1 -v db="'$1'" < sql/populate_levels_from_file.sql

echo 'C. Zone Generation'
echo '1. Updating zones of edges and vertices for all levels'
time ./p_c_u $1 $2
echo '2. Updating all nodes with their nearest skeletal node at every level'
psql -U postgres $1 -v num_levels=$2 < sql/nearest_skeletal_node.sql

echo 'D. Assertions'
psql -U postgres $1 -v num_levels=$2 < sql/assertions.sql

