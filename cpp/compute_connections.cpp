
#include <iostream>
#include <set>
#include <boost/config.hpp>

#include "geometries.h"


int main(int argc, char const *argv[])
{

	std::string file_name(argv[1]);
	int num_levels, nearest_comp;

	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;


	GGraph g;
	std::map<long int, GGraph::vertex_descriptor> id_to_V;
	std::map<long int, GGraph::edge_descriptor> id_to_E;
	// Constructing the whole graph
	int flag = construct_graph_with_geometry(file_name, ',', g, id_to_V, id_to_E);
	if (flag != 1) {
		std::cout << "Error in graph construction" << std::endl;
		return 0;
	}

	//std::cout << "Original Graph: " << std::endl;
	//print_geom_graph(g);
	//std::cout << "Num of edges: " << num_edges(g) << std::endl;
	//std::cout << "Num of vertices: " << num_vertices(g) << std::endl;
	std::vector<Connection> connections;
	get_connections_at_levels(g, id_to_V,
			connections, num_levels);

	//std::cout << "Connections " << std::endl;
	for (int i = 0; i < connections.size(); ++i) {
		std::cout 
		<< connections[i].source
		<< ", " << connections[i].target
		<< ", " << connections[i].level
		<< std::endl;
	}
	



	return 0;
}
