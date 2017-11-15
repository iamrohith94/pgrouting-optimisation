mpic++ -o p_b cpp/parallel_betweenness.cpp -I/usr/local/include/ -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization
#g++ -o s_b cpp/sequential_betweenness.cpp -I/usr/local/include/ -lboost_graph  -lboost_system 
#./s_b $1
mpirun -n $2 ./p_b $1
#sudo su postgres
#psql $1 -v name=\'$1\'
#psql $1 < preprocess.sql
#psql $1 < populate_levels.sql
#psql $1 < populate_components.sql 
#exit