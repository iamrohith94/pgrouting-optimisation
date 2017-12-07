#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>    // for sqrt
#ifndef TYPES_H
#define TYPES_H


#include "types.h"

#endif 

template < class G >
class Dijkstra {
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
 	
	 // exception for termination

 	// visitor that terminates when we find the goal

 	class dijkstra_one_goal_visitor : public boost::default_dijkstra_visitor
 	{
 	public:
 	  explicit dijkstra_one_goal_visitor(V goal) : m_goal(goal) {}
 	  template <class Graph>
 	  void examine_vertex(V u, Graph& g) {

 	    if(u == m_goal){

 	  	  //std::cout << "curr_vertex: " << g[u].id << std::endl;
 	      throw found_goal();
 	    }
 	  }
 	private:
 	  V m_goal;
 	};


 	// find path
 	bool do_dijkstra(G &g, 
 		V s, 
 		V t) {
 		bool found = false;
 		try {
 			// Call A* named parameter interface
 			clear();
 			predecessors.resize(num_vertices(g));
 			distances.resize(num_vertices(g));
         	//std::cout << "starting dijkstra " << std::endl;
         	//std::cout << "source: " << g[s].id << ", target: " << g[t].id << std::endl;
 			boost::dijkstra_shortest_paths(
 					g, s,
 			        boost::predecessor_map(&predecessors[0])
 			        .weight_map(get(&EdgeProperties::weight, g))
 			        .distance_map(&distances[0])
 			        .visitor(dijkstra_one_goal_visitor(t)));
 		}
 		catch(found_goal &) {
			 //std::cout << "dijkstra done " << std::endl;
             return true;  // Target vertex found

         } catch (boost::exception const& ex) {
             (void)ex;
             throw;
         } catch (std::exception &e) {
             (void)e;
             throw;
         } catch (...) {
             throw;
         }
         return true;
         //std::cout << "path found? " << found << std::endl;
 	}

 	E get_edge(G &g, V s, V t) {
 		EO_i out, out_end;
 		for (boost::tie(out, out_end) = out_edges(s, g);
 		        out != out_end; ++out) {
 		    if (boost::target(*out, g) == t) {
 		    	return *out;
 		    }
 		}
 		assert(false);
 		return *out;
 	}

 	void get_path(G &g, 
 		V source,
 		V target,
 		std::vector<V>& vertices) {
 		vertices.clear();
 		if (target == predecessors[target])
 			return;
 		V temp = target;
 		while (temp != predecessors[temp]) {
 			vertices.push_back(temp);
 			temp = predecessors[temp];
 		}
 		vertices.push_back(temp);
 		std::reverse( vertices.begin(), vertices.end() );
 	}



 	void get_path(G &g, 
 		V source,
 		V target,
 		std::vector<E>& path_edges) {
 		std::vector<V> path_vertices;
 		path_edges.clear();
 		get_path(g, source, target, path_vertices);
 		//std::cout << "done with vertex path" << std::endl;
 		//std::cout << "path size: " << path_vertices.size() << std::endl;
 		int limit = path_vertices.size()-1;
 		//std::cout << "i limit: " << limit << std::endl;
 		E e;
 		for (int i = 0; i < limit; ++i) {
 			//std::cout << "lol" << std::endl;
 			e = get_edge(g, path_vertices[i], path_vertices[i+1]);
 			path_edges.push_back(e);
 		}
 		//std::cout << "done" << std::endl;
 	}
 };


