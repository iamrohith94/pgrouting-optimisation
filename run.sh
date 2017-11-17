#Creating necessary schema
psql -U postgres $1 -v num_levels=$3 < sql/preprocess.sql

#Generating betweenness values
mpic++ -o p_b cpp/parallel_betweenness.cpp -I/usr/local/include/ -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization
mpirun -n $2 ./p_b $1

#Updating the db with betweenness values
psql -U postgres $1 -v file_name=$1 < sql/populate_betweenness.sql

#Updating the db with level values
mpic++ -o p_l cpp/parallel_level_updation.cpp -I/usr/local/include/ -lpq -lpqxx -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization
for i in `seq $3`;
do
	mpirun -n $2 ./p_l $1 $i
done

#Creating index on level and promoted_level columns
psql -U postgres $1 < sql/level_indices.sql 

#Updating the db with component values
mpic++ -o p_c cpp/parallel_comp_updation.cpp -I/usr/local/include/ -lpq -lpqxx -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization
mpirun -n $2 ./p_c $1 $3

#Asserting the columns in the table
psql -U postgres $1 -v num_levels=$3 < sql/assertions.sql 

