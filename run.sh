# $1-> dbname $2-> num_levels $3->num_grids $4->exponent 

dbname=$1
num_levels=${2:-10}
num_grids=${3:-4}
exponent=${4:-2}

echo 'Database: '
echo $dbname
echo 'Number of levels: '
echo $num_levels
echo 'Number of grids: '
echo $num_grids
echo 'Exponent: '
echo $exponent

bash compile.sh

echo 'A. Preprocessing'
echo '1. Network Cleaning'
#psql -U postgres $dbname -v num_levels=$num_levels < sql/preprocess.sql
echo '2. Storing Graph in a csv file'
#psql -U postgres -d $dbname -t -A -F"," -c "select id, source, target, cost from cleaned_ways" > "./data/$dbname.csv"
echo '3. Network Division into Grids'
#bash grid-division/grids.sh $dbname $num_grids

echo 'Edge Priority and Levels'
echo '1. Creating a function that fetches vertices grid wise'
#psql -U postgres $dbname < sql/betweenness_st_pairs.sql
echo '2. Computing priority using grid based approach'
time mpirun -n 1 time ./g_b $dbname $num_levels $exponent > "data/${dbname}_geom_levels_out.txt"
echo '3. Update priority and level of edges in edge table'
psql -U postgres $dbname -v db="'$dbname'"  < sql/populate_betweenness.sql


echo 'B. Skeleton Generation'
echo '1. Store graph with geometries in a csv file'
psql -U postgres -d $dbname -t -A -F"," -c "select id, source, target, cost, x1, y1, x2, y2, level from cleaned_ways" > "./data/${dbname}_geom.csv"
echo '2. Computing connections to make skeleton strongly connected at each level'
time ./c_c "${dbname}_geom" $num_levels
echo '3. Computing paths between the above calculated connections'
time mpirun -n 8 time ./p_c "${dbname}_geom" "${dbname}_geom_conns" > "data/${dbname}_pedges_out.txt"
echo '4. Updating the priority and level of edges in the above paths'
psql -U postgres $dbname -v db="'$dbname'" < sql/populate_levels_from_file.sql

echo 'C. Zone Generation'
echo '1. Updating zones of edges and vertices for all levels'
time ./p_c_u $dbname $num_levels
echo '2. Creating index on component ids for all levels'
psql -U postgres $dbname -v num_levels=$num_levels < sql/create_component_indices.sql
#echo '3. Updating all nodes with their nearest skeletal node at every level'
#psql -U postgres $dbname -v num_levels=$num_levels < sql/nearest_skeletal_node.sql

echo 'D. Assertions'
psql -U postgres $dbname -v num_levels=$num_levels < sql/assertions.sql


echo 'E. Plots'
echo '1. Skeleton Size Plot'
python plots/skeleton_size_plot.py $dbname $num_levels
echo '2. Zone Size Plot'
python plots/comp_size_plot.py $dbname $num_levels

