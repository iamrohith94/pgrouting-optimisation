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
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <assert.h> 
#include "helper.h"
#include "strtk.hpp"
#include "dijkstra.h"
#include <pqxx/pqxx> 
#include <boost/format.hpp>
/*
Converts a string of format {v1, v2, ..., vn}
to a vector
*/
void get_vector_from_string(std::string array_string, std::vector<long int> &v) {
	array_string.erase(0, 1);
	array_string.erase(array_string.size() - 1);
	strtk::parse(array_string,",",v);
}

void get_betweenness_vertices_from_db(std::string dbname, 
	std::string vertex_table,
	int p, std::vector<long int>& v) {
	
	std::string conn_str = "dbname = %s user = postgres password = postgres \
	hostaddr = 127.0.0.1 port = 5432";
	conn_str = (boost::format(conn_str) % dbname).str().c_str();
	std::string array_sql = "SELECT * FROM grid_betweenness_pairs(%s, %s)";
	
	
	try {
		pqxx::connection C(conn_str.c_str());
		if (C.is_open()) {
			pqxx::work N(C);
			array_sql = (boost::format(array_sql) 
			%N.quote(vertex_table) %p).str();
			pqxx::result R2( N.exec( array_sql.c_str() ));
			for (pqxx::result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
				get_vector_from_string(c[0].as<std::string>(), v);
			}
		} else {
			std::cout << "Can't open database" << std::endl;
			//return -1;
		}
		C.disconnect ();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		//return -1;
	}
}

void get_pairs_from_vertices(std::vector<long int> v, 
	std::vector<std::pair<long int, long int> >& pairs) {
	for (int i = 0; i < v.size(); ++i) {
		for (int j = 0; j < v.size(); ++j) {
			if(v[i] != v[j]) {
				pairs.push_back(std::make_pair(v[i], v[j]));
			}
		}	
	}
}

void get_dijkstra_paths(Graph &g,
	std::map<long int, Graph::vertex_descriptor>& id_to_V,
	std::map<long int, Graph::edge_descriptor>& id_to_E,
	std::vector<std::pair<long int, long int> >& st_pairs, 
	std::vector<long int>& individual_betweenness, int process_id) {
	Dijkstra<Graph> d;
	std::vector<E> path_edges;
	for (int i = 0; i < st_pairs.size(); ++i) {
		//std::cout << "source: " << st_pairs[i].first << ", target: " << st_pairs[i].second << std::endl;

		assert(d.do_dijkstra(g, id_to_V[st_pairs[i].first]
			, id_to_V[st_pairs[i].second]));
		d.get_path(g, 
			id_to_V[st_pairs[i].first]
			, id_to_V[st_pairs[i].second],
			path_edges);
		
		for (int j = 0; j < path_edges.size(); ++j) {
			//std::cout << "(" << g[path_edges[j]].source << ", " 
			//<< g[path_edges[j]].target << "), process: " << process_id << std::endl; 
			++individual_betweenness[g[path_edges[j]].idx];
		}

	}
}

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cout << "Enter file name" << std::endl;
		return 0;
	}
	string dbname = argv[1]; 
	int num_levels = std::atoi(argv[2]);
	int exponent = std::atoi(argv[3]);

	std::string vertex_table = "cleaned_ways_vertices_pgr";
	std::vector<long int> vertices;
	std::vector<std::pair<long int, long int> > st_pairs;
	std::string s;
	int flag;
	double start_t, end_t;
	
	Graph g;
	std::map<long int, Graph::vertex_descriptor> id_to_V;
	std::map<long int, Graph::edge_descriptor> id_to_E;
	//std::cout << "v table: " << vertex_table << std::endl;
	
	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;
	typedef boost::graph::distributed::mpi_process_group process_group_type;
	process_group_type pg;

	//Constructing the graph from csv file
	flag = construct_graph_from_file(dbname, ',', g, id_to_V, id_to_E);
	std::cout << "Graph Constructed by process: " << world.rank() << std::endl;

	if (flag == 1) {

		get_betweenness_vertices_from_db(dbname, vertex_table, exponent, vertices);
		#if 0
		std::cout << "Vertices: " << std::endl;
		for (int i = 0; i < vertices.size(); ++i) {
			std::cout << vertices[i] << ", ";
		}
		std::cout << std::endl;
		#endif
		get_pairs_from_vertices(vertices, st_pairs);
		std::cout << "Vertices: " << vertices.size() << std::endl;
		std::cout << "ST Pairs: " << st_pairs.size() << std::endl;
		/*
		boost::archive::text_oarchive oa(oss);
		oa << flag << g << id_to_E << id_to_V << st_pairs;
		s = oss.str(); 
		//Sending the graph to all other processes
		boost::mpi::broadcast(world, s, 0);
		*/
	}
	else {
		std::cout << "Error printing graph" << std::endl;
		return 0;
	}

	int bucket_size = st_pairs.size()/world.size();
	int start = world.rank()*bucket_size;
	int end = (st_pairs.size() < start+bucket_size) ? st_pairs.size() : start+bucket_size;

	//std::cout << "start: " << start << ", end: " << end 
	//		<< ", process: " << world.rank() << std::endl;
	std::vector<long int> individual_betweenness(boost::num_edges(g),0);
	std::vector<long int> total_betweenness(boost::num_edges(g),0);
	std::vector<std::pair<long int, long int> > st_pairs_process(st_pairs.begin()+start, st_pairs.begin()+end);
	#if 0
	std::cout << "ST Pairs: " << st_pairs_process.size() 
		<< ", process: " << world.rank() << std::endl;
	for (int i = 0; i < st_pairs_process.size(); ++i) {
		std::cout << "(" << st_pairs_process[i].first 
		<< ", " << st_pairs_process[i].second << ") "
		<< ", process: " << world.rank() << ", ";
	}
	std::cout << std::endl;
	#endif
	/* Dijkstra on the st_pairs in a process */
	//std::cout << "starting dijkstra "<< std::endl;
	start_t = MPI_Wtime();
	get_dijkstra_paths(g, id_to_V, id_to_E, st_pairs_process, individual_betweenness, world.rank());
	end_t = MPI_Wtime();
	std::cout << "Wall clock taken for process " << world.rank() << " : " << end_t - start_t << std::endl;
	#if 0
	std::cout << "Betweenness values: " << individual_betweenness.size() 
			<< ", process: " << world.rank() << std::endl;

	for (int i = 0; i < individual_betweenness.size(); ++i) {
		std::cout << "(" << g[id_to_E[i]].source 
        	<< ", " << g[id_to_E[i]].target << "): " << individual_betweenness[i]
		<< ", process: " << world.rank() << std::endl;
	}	
	#endif

	if (world.rank() == 0) {
        boost::mpi::reduce(world, &individual_betweenness.front(), boost::num_edges(g), &total_betweenness.front(), std::plus<int>(), 0);
    	#if 0
        std::cout << "Final Betweenness values: " << total_betweenness.size() 
        		<< ", process: " << world.rank() << std::endl;

        for (int i = 0; i < total_betweenness.size(); ++i) {
        	std::cout << "(" << g[id_to_E[i]].source 
        	<< ", " << g[id_to_E[i]].target << "): " << total_betweenness[i]
        	<< ", process: " << world.rank() << std::endl;
        }
        #endif
    	int dump_flag = dump_to_file_group_by_id(g, id_to_E, total_betweenness, num_levels, dbname, ',');
    	//std::cout << "Dump flag: " << dump_flag << std::endl;
    	if(dump_flag != -1) {
    		std::cout << "Dump succeeded" << std::endl;
    	}
    	else {
    		std::cout << "Dump failed" << std::endl;	
    	}

    } else {
        boost::mpi::reduce(world, &individual_betweenness.front(), boost::num_edges(g), std::plus<int>(), 0);
    }
   

	return 0;
}
