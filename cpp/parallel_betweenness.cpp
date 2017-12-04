#include <iostream>
#include <vector>
#include <stack>
#include <boost/graph/use_mpi.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/betweenness_centrality.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/set.hpp> 
#include "helper.h"


int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cout << "Enter file name" << std::endl;
		return 0;
	}

	std::string file_name(argv[1]);
	int num_levels;
	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;
	//int num_sources = std::atoi(argv[2]);

	std::stack<V> source_vertices;
	std::set<V> source_indexes;

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;
	typedef boost::graph::distributed::mpi_process_group process_group_type;
	process_group_type pg;
	Graph g;
	std::map<long int, Graph::vertex_descriptor> id_to_V;
	std::map<long int, Graph::edge_descriptor> id_to_E;
	double start, end;
	int flag;


	if (world.rank() == 0) {
		std::string s; 
		std::ostringstream oss;
		//Constructing the graph from csv file
		flag = construct_graph_from_file(file_name, ',', g, id_to_V, id_to_E);
		if (flag == 1) {
			get_random_sources(g, source_indexes);
			boost::archive::text_oarchive oa(oss);
			oa << flag << g << id_to_E << id_to_V << source_indexes;
			s = oss.str(); 
			//Sending the graph to all other processes
			boost::mpi::broadcast(world, s, 0);
		}
	}
	else {

		std::string s;
		//Receiving the graph from the master
		boost::mpi::broadcast(world, s, 0);
		std::istringstream iss(s);
		boost::archive::text_iarchive ia(iss);
		ia >> flag;
		ia >> g;
		ia >> id_to_E;
		ia >> id_to_V;
		ia >> source_indexes;
	}
	
	/* Output the graph */
	//print_graph(g);

	std::vector<double> e_centrality(boost::num_edges(g));

	/*Construct graph from database*/
	if(flag == 1) {
		//std::cout << "rank: " << world.rank() << std::endl;
		//Extract the vertex descriptors of the sources into a queue

		/*
		std::cout << "sources: " << std::endl;
		for (std::set<V>::iterator it=source_indexes.begin(); it!=source_indexes.end(); ++it) {
		    source_vertices.push(*(vertices(g).first+*it));
		    std::cout << "desC: " << source_vertices.top() << ", id: " << g[source_vertices.top()].id << std::endl;
		}*/

		
		
		typedef boost::iterator_property_map<typename std::vector<double>::iterator,
		typename boost::property_map<Graph, long int EdgeProperties::*>::type,
		double, double&> ECentralityMap;


		ECentralityMap e_centrality_property =
		make_iterator_property_map(e_centrality.begin(), get(&EdgeProperties::idx, g));
		
		start = MPI_Wtime();
		
		boost::non_distributed_brandes_betweenness_centrality(pg, g,  
			boost::edge_centrality_map(e_centrality_property)
			.weight_map(get(&EdgeProperties::weight, g)).buffer(source_vertices));
			
		
		
		end = MPI_Wtime();
		std::cout << "Wall clock taken for process " << world.rank() << " : " << end - start << std::endl;
		//std::cout << "Hi from process " << world.rank() << " of " << world.size() << std::endl;

		if (process_id(pg) == 0) {
			/*
			std::cout << "Time taken for process in master: " << end - start << std::endl;
			std::cout << "Edge Centrality values " << std::endl;
			for (int i = 0; i < e_centrality.size(); ++i) {
				std::cout << "id: " << g[id_to_E[i]].id << ", "
				<< "source: " << g[id_to_E[i]].source << ", "
				<< "target: " << g[id_to_E[i]].target << ", " 
				<< "betweenness: " << e_centrality[i] << std::endl;
			}
			std::cout << "Starting dump... " << std::endl;
			*/
			std::vector<int> level;
			get_levels(e_centrality, num_levels, level);
			//std::cout << "e_centrality size " << e_centrality.size() << std::endl;
			//std::cout << "level size " << level.size() << std::endl;
			assert(e_centrality.size() == level.size());
			int dump_flag = dump_to_file(g, id_to_E, e_centrality, level, num_levels, "out_"+file_name, ',');
			//std::cout << "Dump flag: " << dump_flag << std::endl;
			if(dump_flag != -1) {
				std::cout << "Dump succeeded" << std::endl;
			}
			else {
				std::cout << "Dump failed" << std::endl;	
			}
			
		}
		
	}
	else {
		std::cout << "Graph construction failed" << std::endl;
	}
	return 0;

}