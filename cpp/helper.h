#include <boost/format.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <stdio.h>

#include "ranker.h"

#include "types.h"
//#include <boost/lexical_cast.hpp>


typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperties, EdgeProperties> Graph;

typedef  boost::graph_traits < Graph >::vertex_descriptor V;
typedef  boost::graph_traits < Graph >::vertex_iterator V_i;
typedef  boost::graph_traits < Graph >::edge_iterator E_i;
typedef boost::graph_traits < Graph >::out_edge_iterator EO_i;
typedef boost::graph_traits < Graph >::in_edge_iterator IO_i;


template<class G>
void print_graph(G &g) {
	typedef  typename boost::graph_traits < G >::vertex_iterator V_i;

	typedef typename boost::graph_traits < G >::out_edge_iterator EO_i;
	V_i vi;
	EO_i out, out_end;
	/* Vertices
	std::cout << "Vertices" << std::endl;
	for (vi = vertices(g).first;
                 vi != vertices(g).second; ++vi) {
		std::cout << g[*vi].id << ", ";
    }
    std::cout << std::endl;
	*/
    //Edges

	std::cout << "Edges" << std::endl;
    for (vi = boost::vertices(g).first;
            vi != boost::vertices(g).second; ++vi) {
        std::cout << g[*vi].id << ": " << " out_edges_of(" << g[(*vi)].id << "):";
        for (boost::tie(out, out_end) = out_edges(*vi, g);
                out != out_end; ++out) {
            std::cout << ' '
                << g[*out].id << "=("
                << g[*out].source << ", "
                << g[*out].target << ") = "
                << g[*out].weight <<"\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}




int construct_graph_from_file(std::string file_name, const char delimiter, Graph &g, std::map<long int,
	Graph::vertex_descriptor>& id_to_V,
	std::map<long int, Graph::edge_descriptor>& id_to_E) {
	std::ifstream file(("./data/"+file_name+".csv").c_str()); // pass file name as argment
	std::string linebuffer;
	long int eid, source, target;
	double cost, reverse_cost;
	long int edge_count = 0;
	std::pair<Graph::edge_descriptor, bool> p;
	while (file && getline(file, linebuffer)){
		if (linebuffer.length() < 1)continue;
		else {
			std::vector<std::string> result;
			std::stringstream ss(linebuffer);
			std::string token;
			while (std::getline(ss, token, delimiter)) {
			    result.push_back(token);
			}
			if (result.size() < 4)
				continue;
			/*
			eid = boost::lexical_cast<long int>(result[0].c_str());
			source = boost::lexical_cast<long int>(result[1].c_str());
			std::cout << "s " << source << std::endl;
			target = boost::lexical_cast<long int>(result[2].c_str());
			cost = boost::lexical_cast<double>(result[3].c_str());
			*/
			
			//std::cout << "s " << result.size() << std::endl;
			
			eid = std::atol(result[0].c_str());
			source = std::atol(result[1].c_str());
			target = std::atol(result[2].c_str());
			cost = std::atof(result[3].c_str());

			if (result.size() == 5)
				reverse_cost = std::atof(result[4].c_str());
				//reverse_cost = boost::lexical_cast<double>(result[4].c_str());
				
			else
				reverse_cost = -1.000;

			
			if (id_to_V.find(source) == id_to_V.end()) {
				id_to_V[source] = boost::add_vertex(g);
				g[id_to_V[source]].id = source;
			}
			if (id_to_V.find(target) == id_to_V.end()) {
				id_to_V[target] = boost::add_vertex(g);
				g[id_to_V[target]].id = target;
			}
			if (cost >= 0.00000) {
				
				p = boost::add_edge(id_to_V[source], id_to_V[target], g);
				g[p.first].weight = cost;
				g[p.first].source = source;
				g[p.first].target = target;
				id_to_E[edge_count] = p.first;
				g[p.first].idx = edge_count++;
				g[p.first].id = eid;
			}
			if (reverse_cost >= 0.00000) {
				p = boost::add_edge(id_to_V[target], id_to_V[source], g);
				g[p.first].weight = reverse_cost;
				g[p.first].source = target;
				g[p.first].target = source;
				id_to_E[edge_count] = p.first;
				g[p.first].idx = edge_count++;
				g[p.first].id = eid; 
			}
			
		}

	}
	return 1;
	
}







int get_levels(std::vector<double> values, int num_levels, std::vector<int>& level) {
    int bucket_size = 100/num_levels, interval_size, prev;
	std::vector<double> caps;

	for (int i = 0; i <= num_levels; ++i) {
		//caps.push_back(quantile(values, (i*bucket_size*1.000)/100));
		caps.insert(caps.begin(), quantile(values, (i*bucket_size*1.000)/100)); 
	}
	//Adding 1 to the highest betweenness value
	caps[0] += 1;
	//Adding -1 to the lowest betweenness value
	caps[caps.size()-1] -= 1;


	/*
	std::cout << "Intervals" << std::endl;
	for (int i = 0; i < caps.size(); ++i) {
		std::cout << caps[i] << ", ";
	}
	*/
	
	for (int i = 0; i < values.size(); ++i) {
		//std::cout << "b value: " << values[i] << std::endl;
		/* gets the bucket to which the value belongs
		TODO: convert this to binary search */
		for (int k = 0; k < caps.size()-1; k++) {
			if ((values[i] - caps[k+1] > 0.0000000001)
				&& (values[i] - caps[k] <= 0.0000000001)) {
				//std::cout << "value: " << k << std::endl;
				//std::cout << "caps[k]: " << caps[k] << ", "
				//<< "caps[k+1]: " << caps[k+1] << std::endl;
				level.push_back(k+1);
				break;
			}
		}
	}
	return 1;
}


int get_random_sources(Graph &g, std::set<V>& indexes) {
	size_t max_index = num_vertices(g);
	int random_index;
	while (indexes.size() < sqrt(max_index)) {
		indexes.insert(rand() % max_index);
	}
	return 1;
}


int dump_to_file(const Graph &g, std::map<long int, Graph::edge_descriptor>& id_to_E,
	const std::vector<double> edge_centrality, std::vector<int>& level, int num_levels,
	std::string output_file, const char delimiter) {
	std::ofstream myfile;
	/*
	std::string comp_init="";
	for (int j = 0; j < num_levels; ++j) {
		if (j < num_levels-1)
			comp_init +=  "1, ";
		else
			comp_init += "1";
	}
	*/
	get_levels(edge_centrality, num_levels, level);
    myfile.open (("./data/"+output_file+"_columns.csv").c_str());

    for (int i = 0; i < edge_centrality.size(); ++i) {
    	myfile << g[id_to_E[i]].id << delimiter
    	<< g[id_to_E[i]].source << delimiter
    	<< g[id_to_E[i]].target << delimiter
    	<< edge_centrality[i] << delimiter 
    	<< level[i] << std::endl;
    	//<< comp_init << std::endl;
    }
    myfile.close();
    return 1;
}
