echo 'Compiling Grid Betweenness code......'
mpic++ -o g_b cpp/grid_betweenness.cpp -I/usr/local/include/ -lboost_graph_parallel -lpq -lpqxx -lboost_mpi -lboost_system -lboost_serialization -std=c++11

echo 'Compiling Connections code......'
g++ -o c_c cpp/compute_connections.cpp -std=c++11

echo 'Compiling Parallel Path Computation......'
mpic++ -o p_c cpp/parallel_path_comp.cpp -I/usr/local/include/ -lpq -lpqxx -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization -std=c++11

echo 'Compiling Component Updation code......'
mpic++ -o p_c_u cpp/parallel_comp_updation.cpp -I/usr/local/include/ -lpq -lpqxx -lboost_graph_parallel -lboost_mpi -lboost_system -lboost_serialization
