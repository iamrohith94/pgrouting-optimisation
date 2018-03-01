
#include <iostream>
#include <set>
#include <stack>
#include <boost/config.hpp>

#include "geometries.h"

void dump_to_file(std::string output_file, 
	std::vector<Connection>& connections,
	const char delimiter) {
	std::ofstream myfile;
	myfile.open (("./data/"+output_file+"_conns.csv").c_str());
	for (int i = 0; i < connections.size(); ++i) {
		myfile 	<< connections[i].source << delimiter
		<< connections[i].target << delimiter
		<< connections[i].level
		<< std::endl; 
		//<< ", " << world.rank() << std::endl;
	}
    myfile.close();
}


void reorder_connections_by_level(std::vector<Connection>& connections,
	int num_levels) {
	std::map<int, std::stack<Connection> > level_connections;
	std::map<int, std::stack<Connection> >::iterator it;
	long int total_connections = connections.size();
	for (int i = 0; i < connections.size(); ++i) {
		level_connections[connections[i].level].push(connections[i]);
	}
	connections.clear();
	while(total_connections > 0) {
		for (it = level_connections.begin(); it != level_connections.end()
			&& total_connections > 0; ++it) {
			if (it->second.size() > 0) {
				connections.push_back(it->second.top());
				it->second.pop();
				total_connections--;
			}
		}
	}
}

int main(int argc, char const *argv[])
{

	std::string file_name(argv[1]);
	int num_levels, nearest_comp;

	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;

	// /std::cout << "arg 1 " << file_name << std::endl;


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

	reorder_connections_by_level(connections, num_levels);

	/*std::cout << "Connections " << std::endl;
	for (int i = 0; i < connections.size(); ++i) {
		std::cout 
		<< connections[i].source
		<< ", " << connections[i].target
		<< ", " << connections[i].level
		<< std::endl;
	}*/
	dump_to_file(file_name, connections, ',');



	return 0;
}
