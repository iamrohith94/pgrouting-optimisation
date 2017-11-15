#include <iostream>
#include <vector>
#include <boost/graph/betweenness_centrality.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <fstream>


struct VertexProperties {
	long int id;
};

struct EdgeProperties {
	long int id;
	long int idx;
	long int source;
	long int target;
	double weight;

};





typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProperties, EdgeProperties> Graph;

typedef  boost::graph_traits < Graph >::vertex_iterator V_i;
typedef  boost::graph_traits < Graph >::edge_iterator E_i;
typedef boost::graph_traits < Graph >::out_edge_iterator EO_i;



void print_graph(Graph &g) {
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

int main(int argc, char* argv[]) {

	if (argc < 2) {
		std::cout << "Enter file name" << std::endl;
		return 0;
	}

	std::string file_name(argv[1]);
	Graph g;
	std::map<long int, Graph::vertex_descriptor> id_to_V;
	std::map<long int, Graph::edge_descriptor> id_to_E;
	double start, end;
	int flag = construct_graph_from_file(file_name, ',', g, id_to_V, id_to_E);
	print_graph(g);

	std::vector<double> e_centrality(boost::num_edges(g));

	/*Construct graph from database*/
	if(flag == 1) {
		//std::cout << "rank: " << world.rank() << std::endl;
		
		typedef boost::iterator_property_map<typename std::vector<double>::iterator,
		typename boost::property_map<Graph, long int EdgeProperties::*>::type,
		double, double&> ECentralityMap;


		ECentralityMap e_centrality_property =
		make_iterator_property_map(e_centrality.begin(), get(&EdgeProperties::idx, g));
		
		boost::brandes_betweenness_centrality(g,  
			boost::edge_centrality_map(e_centrality_property)
			.weight_map(get(&EdgeProperties::weight, g)));
		//std::cout << "Wall clock taken for process " << world.rank() << " : " << end - start << std::endl;
		//std::cout << "Hi from process " << world.rank() << " of " << world.size() << std::endl;

			
		std::cout << "Time taken for process in master: " << end - start << std::endl;
		std::cout << "Edge Centrality values " << std::endl;
		for (int i = 0; i < e_centrality.size(); ++i) {
			std::cout << "id: " << g[id_to_E[i]].id << ", "
			<< "source: " << g[id_to_E[i]].source << ", "
			<< "target: " << g[id_to_E[i]].target << ", " 
			<< "betweenness: " << e_centrality[i] << std::endl;
		}
		/*
		std::cout << "Starting dump... " << std::endl;
		
		std::vector<int> level;
		get_levels(e_centrality, 10, level);
		std::cout << "e_centrality size " << e_centrality.size() << std::endl;
		std::cout << "level size " << level.size() << std::endl;
		assert(e_centrality.size() == level.size());
		
		int dump_flag = dump_to_file(g, id_to_E, e_centrality, level, "out_"+file_name, ',');
		//std::cout << "Dump flag: " << dump_flag << std::endl;
		if(dump_flag != -1) {
			std::cout << "Dump succeeded" << std::endl;
		}
		else {
			std::cout << "Dump failed" << std::endl;	
		}*/
			
		
	}
	else {
		std::cout << "Graph construction failed" << std::endl;
	}
	return 0;

}