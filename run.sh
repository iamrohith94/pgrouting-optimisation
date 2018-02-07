

echo 'C. Zone Generation'
echo '1. Updating zones of edges and vertices for all levels'
time ./p_c_u $1 $2
echo '2. Updating all nodes with their nearest skeletal node at every level'
psql -U postgres $1 -v num_levels=$2 < sql/nearest_skeletal_node.sql

echo 'D. Assertions'
psql -U postgres $1 -v num_levels=$2 < sql/assertions.sql

