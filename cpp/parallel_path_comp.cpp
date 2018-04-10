
#include <iostream>
#include <set>
#include <boost/config.hpp>
#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/set.hpp> 
#include "geometries.h"
#include <cstdlib>


int line_count(std::string file_name) {
	int count = 0;
	std::string line;
	std::ifstream myfile(file_name.c_str());
	while (std::getline(myfile, line))
	    ++count;
	//std::cout << "line_count: " << count << std::endl;
	return count;
}

void get_process_connections(std::string file_name, 
	std::vector<Connection>& connections,
	int process_id, int num_processes, const char delimiter) {
	connections.clear();
	std::string line;
	int curr = 0;
	Connection temp;
	std::ifstream myfile(file_name.c_str());
	int num_lines = line_count(file_name);
	int bucket_size;
	if (num_lines >= num_processes) {
	 	bucket_size = num_lines/num_processes;
	}
	else 
		bucket_size = num_lines/num_processes +1;
	int start = process_id*bucket_size;
	int end;
	if (process_id == num_processes-1) {
		end = num_lines;
	}
	else {
		end = start + bucket_size;
	}
	/*
	if (end + bucket_size >= num_lines && end < num_lines && num_lines > num_processes) {
		end += num_lines%num_processes;
	}*/
	

	//std::cout << start << ", " << end << ", process: " << process_id << std::endl;
	//Getting to the start line
	while (std::getline(myfile, line)) {
		if(curr < start) {
			curr++;
			continue;
		}
	    if (curr < end) {
	    	std::vector<std::string> result;
	    	std::stringstream ss(line);
	    	std::string token;

	    	//std::cout << line << ", " << process_id << std::endl;
	    	while (std::getline(ss, token, delimiter)) {
	    		result.push_back(token);
	    	}
	    	if (result.size() < 3)
	    		continue;
	    	temp.source = std::atol(result[0].c_str());
	    	temp.target = std::atol(result[1].c_str());
	    	temp.level = std::atof(result[2].c_str());
	    	connections.push_back(temp);
	    	
	    	curr++;
	    }
	    else 
	    	break;
	}
}

void dump_to_file(std::string output_file, 
	std::vector<std::vector<PromotedEdge> >& total_promoted_edges,
	const char delimiter) {
	std::ofstream myfile;
	myfile.open (("./data/"+output_file+"_conn_edges.csv").c_str());
	for (int i = 0; i < total_promoted_edges.size(); ++i) {
		for (int j = 0; j < total_promoted_edges[i].size(); ++j) {
			myfile << total_promoted_edges[i][j].id << delimiter
			<< total_promoted_edges[i][j].source << delimiter
			<< total_promoted_edges[i][j].target << delimiter
			<< total_promoted_edges[i][j].level
			<< std::endl; 
		}
	}
    myfile.close();
}


int main(int argc, char *argv[])
{
	if (argc < 3) {
	std::cout << "Enter file name" << std::endl;
	return 0;
	}
	std::string graph_file_name(argv[1]);
	std::string connections_file_name(argv[2]);
	double start, end;
	int num_levels;
	if (argc == 4)
		num_levels = std::atoi(argv[3]);
	else
		num_levels = 10;
	//int num_sources = std::atoi(argv[2]);

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;
	typedef boost::graph::distributed::mpi_process_group process_group_type;
	process_group_type pg;
	GGraph g;
	std::map<long int, GGraph::vertex_descriptor> id_to_V;
	std::map<long int, GGraph::edge_descriptor> id_to_E;
	int flag;

	if (world.rank() == 0) {
		std::string s; 
		std::ostringstream oss;
		//Constructing the graph from csv file
		//std::cout << "graph file: " << graph_file_name << std::endl;
		flag = construct_graph_with_geometry(graph_file_name, ',', g, id_to_V, id_to_E);
		//print_geom_graph(g);
		if (flag == 1) {
			boost::archive::text_oarchive oa(oss);
			oa << flag << g << id_to_E << id_to_V;
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
		//std::cout << "Received graph by " << world.rank() << std::endl;
	}

	std::vector<PromotedEdge> promoted_edges;
	std::vector<Connection> connections;
	get_process_connections("./data/"+connections_file_name+".csv", connections, world.rank(), world.size(), ',');
	std::cout << "Received " << connections.size() << " connnections by " << world.rank() << std::endl;
	start = MPI_Wtime();
	std::cout << "Starting...." << std::endl;
	for (int i = 0; i < connections.size(); ++i) {
		//std::cout << "source: " << connections[i].source << std::endl;
		//std::cout << "target: " << connections[i].target << std::endl;
		get_connecting_edges(g,
			id_to_V,
			connections[i].source, 
			connections[i].target, 
			promoted_edges, 
			connections[i].level);
		#if 0
		std::cout << connections[i].source 
		<< ", " << connections[i].target
		<< ", " << connections[i].level
		<< ", " << world.rank() << std::endl;
		#endif
		
	}
	end = MPI_Wtime();
	std::cout << "Wall clock taken for process " << world.rank() << " : " << end - start << std::endl;
	//std::cout << "Computed connnection edges by " << world.rank() << std::endl;	
	//std::cout << "Promoted edges of Process " << world.rank() << std::endl;
	#if 0
	for (int i = 0; i < promoted_edges.size(); ++i) {
		std::cout << promoted_edges[i].id << ", "
		<< promoted_edges[i].source << ", "
		<< promoted_edges[i].target << ", "
		<< promoted_edges[i].level
		<< std::endl; 
		//<< ", " << world.rank() << std::endl;
	}
	#endif
	/*
	Gathering the path between connections from all the processes
	*/
	if (world.rank() == 0) {
		std::vector<std::vector<PromotedEdge> > total_promoted_edges;
		boost::mpi::gather(world, promoted_edges, total_promoted_edges, 0);
		dump_to_file(graph_file_name, total_promoted_edges, ',');
	}
	else {
		boost::mpi::gather(world, promoted_edges, 0);	
	}


	
	return 0;
}
