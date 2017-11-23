#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>    // for sqrt

#include "types.h"

template < class G >
class Pgr_astar {
 public:
 	typedef typename boost::graph_traits < G >::vertex_descriptor V;
 	typedef typename boost::graph_traits < G >::edge_descriptor E;

 	std::vector< V > predecessors;
 	std::vector< double > distances;

 	struct found_goal {};

 	void clear() {
         predecessors.clear();
         distances.clear();
     }
 	// euclidean distance heuristic
 	class distance_heuristic : public boost::astar_heuristic<G, double>
 	{
 	public:
 	  distance_heuristic(G &g, V goal)
 	    : m_g(g), m_goal(goal) {}
 	  double operator()(V u) {
 	    double dx = m_g[m_goal].x - m_g[u].x;
 	    double dy = m_g[m_goal].y - m_g[u].y;
 	    return ::sqrt(dx * dx + dy * dy);
 	  }
 	private:
 	  V m_goal;
 	  G m_g;
 	};
	 // exception for termination

 	// visitor that terminates when we find the goal
 	class astar_goal_visitor : public boost::default_astar_visitor
 	{
 	public:
 	  explicit astar_goal_visitor(V goal) : m_goal(goal) {}
 	  template <class Graph>
 	  void examine_vertex(V u, Graph& g) {

 	  	//std::cout << "curr_vertex: " << g[u].id << std::endl;
 	    if(u == m_goal)
 	      throw found_goal();
 	  }
 	private:
 	  V m_goal;
 	};


 	// find path
 	bool do_astar(G &g, 
 		V source, 
 		V target) {
 		bool found = false;
 		try {
 			// Call A* named parameter interface
 			clear();
 			predecessors.resize(num_vertices(g));
 			distances.resize(num_vertices(g));
         	//std::cout << "starting astar " << std::endl;
         	//std::cout << "source: " << g[source].id << ", target: " << g[target].id << std::endl;
 			boost::astar_search(
 			        g, source,
 			        distance_heuristic(g, target),
 			        boost::predecessor_map(&predecessors[0])
 			        .weight_map(get(&EdgeG::weight, g))
 			        .distance_map(&distances[0])
 			        .visitor(astar_goal_visitor(target)));
 		}
 		catch(found_goal &) {
             found = true;  // Target vertex found
         }
         //std::cout << "path found? " << found << std::endl;
 		return found;
 	}


 	void get_path(G &g, 
 		V source,
 		V target,
 		std::vector<long int>& vertices) {
 		vertices.clear();
 		if (target == predecessors[target])
 			return;
 		V temp = target;
 		while (temp != predecessors[temp]) {
 			vertices.push_back(g[temp].id);
 			temp = predecessors[temp];
 		}
 		vertices.push_back(g[temp].id);
 		std::reverse( vertices.begin(), vertices.end() );
 	}
 };


