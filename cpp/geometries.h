#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <fstream>

namespace bg = boost::geometry;


typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::multi_point<point_t> mpoint_t;
typedef bg::model::polygon<point_t> polygon_t;
struct VertexG {
	long int id;
	double x;
	double y;
};

struct EdgeG {
	long int id;
	long int idx;
	long int source;
	long int target;
	double weight;
	int level;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexG, EdgeG> GGraph;


typedef  boost::graph_traits < GGraph >::vertex_descriptor V_g;
typedef  boost::graph_traits < GGraph >::edge_descriptor E_g;
typedef  boost::graph_traits < GGraph >::vertex_iterator V_i_g;
typedef  boost::graph_traits < GGraph >::edge_iterator E_i_g;
typedef boost::graph_traits < GGraph >::out_edge_iterator EO_i_g;
typedef boost::graph_traits < GGraph >::in_edge_iterator IO_i_g;


template<class G>
void print_geom_graph(G &g) {
	typedef  typename boost::graph_traits < G >::vertex_iterator V_i;

	typedef typename boost::graph_traits < G >::out_edge_iterator EO_i;
	V_i vi;
	EO_i out, out_end;
    std::cout << "Vertices" << std::endl;
    for (vi = boost::vertices(g).first;
            vi != boost::vertices(g).second; ++vi) {
    	std::cout << "id: " << g[*vi].id 
    	<< ", x: " << g[*vi].x
    	<< ", y: " << g[*vi].y
    	<< std::endl;
	} 
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

void add_edge_to_graph(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	int eid, int source, int target, double weight,
	double x_1, double y_1,
	double x_2, double y_2, int level) {
	std::pair<GGraph::edge_descriptor, bool> p;
	if (id_to_V.find(source) == id_to_V.end()) {
		std::cout << "Adding vertex " <<  source << std::endl;
		std::cout << "x: " << x_1 << ", y: " << y_1 << std::endl;
		id_to_V[source] = boost::add_vertex(g);
		g[id_to_V[source]].id = source;
		g[id_to_V[source]].x = x_1;
		g[id_to_V[source]].y = y_1;
	}
	if (id_to_V.find(target) == id_to_V.end()) {
		std::cout << "Adding vertex " <<  target << std::endl;
		id_to_V[target] = boost::add_vertex(g);
		g[id_to_V[target]].id = target;
		g[id_to_V[target]].x = x_2;
		g[id_to_V[target]].y = y_2;

		std::cout << "x: " << g[id_to_V[target]].x << ", y: " << g[id_to_V[target]].y << std::endl;
	}
	if (weight >= 0.00000) {
		
		p = boost::add_edge(id_to_V[source], id_to_V[target], g);
		g[p.first].weight = weight;
		g[p.first].source = source;
		g[p.first].target = target;
		g[p.first].idx = id_to_E.size();
		id_to_E[g[p.first].idx] = p.first;
		g[p.first].id = eid;
		g[p.first].level = level;
	}

}


int construct_graph_with_geometry(std::string file_name, const char delimiter, GGraph &g, std::map<long int,
	GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E) {
	std::ifstream file(("./data/"+file_name+".csv").c_str()); // pass file name as argment
	std::string linebuffer;
	long int eid, source, target;
	double cost, x_1, y_1, x_2, y_2;
	long int edge_count = 0;
	int level;
	std::pair<GGraph::edge_descriptor, bool> p;
	while (file && getline(file, linebuffer)){
		if (linebuffer.length() < 1)continue;
		else {
			std::vector<std::string> result;
			std::stringstream ss(linebuffer);
			std::string token;
			while (std::getline(ss, token, delimiter)) {
			    result.push_back(token);
			}
			if (result.size() < 9)
				continue;
			
			eid = std::atol(result[0].c_str());
			source = std::atol(result[1].c_str());
			target = std::atol(result[2].c_str());
			cost = std::atof(result[3].c_str());
			x_1 = std::atof(result[4].c_str());
			y_1 = std::atof(result[5].c_str());
			x_2 = std::atof(result[6].c_str());
			y_2 = std::atof(result[7].c_str());
			level = std::atof(result[8].c_str());
			add_edge_to_graph(g, id_to_V, id_to_E, 
				eid, source, target, cost, 
				x_1, y_1, x_2, y_2, level);
			
		}

	}
	return 1;
	
}


void print_components(GGraph g, std::vector<int> components) {
	V_i_g vi;
    for (vi = boost::vertices(g).first;
            vi != boost::vertices(g).second; ++vi) {
    	std::cout << "id: " << g[*vi].id << ", comp: " << components[*vi] << std::endl;
    }
}

void print_geometries(std::vector<mpoint_t> comp_geometries) {
	for (int i = 0; i < comp_geometries.size(); ++i) {
		std::cout << "comp: " << i << ", geom: " << bg::dsv(comp_geometries[i]) << std::endl;
	}
}

void get_graph_at_level(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l, int level) {

	V_i_g vi;
	EO_i_g out, out_end;
	long int source, target;

	std::pair<GGraph::edge_descriptor, bool> p;
	/* Adding edges at a given level */
	for (vi = boost::vertices(g).first;
	        vi != boost::vertices(g).second; ++vi) {
	    //std::cout << g[*vi].id << ": " << " out_edges_of(" << g[(*vi)].id << "):";
	    for (boost::tie(out, out_end) = out_edges(*vi, g);
	            out != out_end; ++out) {
	    	if (g[*out].level <= level) {
	    		source = g[*out].source;
	    		target = g[*out].target;
	    		add_edge_to_graph(lg, id_to_V_l, id_to_E_l,
	    			g[*out].id, source, target, g[*out].weight,
	    			g[id_to_V[source]].x, g[id_to_V[source]].y,
	    			g[id_to_V[target]].x, g[id_to_V[target]].y,
	    			g[*out].level);
	    	}
	    }
	}
}



#if 0
void add_edges_to_graph(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l,
	std::vector<int> edge_ids) {
	long int source, target;

	std::pair<GGraph::edge_descriptor, bool> p;
	for (int i = 0; i < edge_ids.size(); ++i) {
		// Add edge to level graph if it doesn not exist
		if (id_to_E_l.find(edge_ids[0]) == id_to_E_l.end()) {
			
			source = g[id_to_E[edge_ids[0]]].source;
			target = g[id_to_E[edge_ids[0]]].target;
			if (id_to_V_l.find(source) == id_to_V_l.end()) {
				id_to_V_l[source] = boost::add_vertex(lg);
				lg[id_to_V_l[source]].id = source;
			}

			if (id_to_V_l.find(target) == id_to_V_l.end()) {
				id_to_V_l[target] = boost::add_vertex(lg);
				lg[id_to_V_l[target]].id = target;
			}
			p = boost::add_edge(id_to_V_l[source], id_to_V_l[target], lg);
			lg[p.first].weight = g[id_to_E[edge_ids[0]]].weight;
			lg[p.first].source = source;
			lg[p.first].target = target;
			lg[p.first].idx = id_to_E_l.size();
			id_to_E_l[lg[p.first].idx] = p.first;
			lg[p.first].id = g[id_to_E[edge_ids[0]]].id;
		}

	}
}
#endif

void get_astar_path(GGraph& bg, V_i_g source, V_i_g target, std::vector<V_g> path) {

}

int get_closest_comp(int comp_1, std::vector<mpoint_t>& comp_geometries) {
	mpoint_t comp_geom = comp_geometries[comp_1];
	int nearest_comp = -1;
	double min_dist = std::numeric_limits<double>::max();
	for (int i = 0; i < comp_geometries.size(); ++i) {
		if (i != comp_1 && bg::distance(comp_geometries[comp_1], 
			comp_geometries[i]) < min_dist) {
			nearest_comp = i;
			min_dist = bg::distance(comp_geometries[comp_1], 
			comp_geometries[i]);		
		}
	}
	return nearest_comp;
}

void get_comp_geom(GGraph& g, std::vector<int> components, std::vector<mpoint_t>& comp_geometries) {
	V_i_g vi;
	comp_geometries.resize(components.size());
	for (vi = boost::vertices(g).first;
	        vi != boost::vertices(g).second; ++vi) {
		bg::append(comp_geometries[components[*vi]], point_t(g[*vi].x, g[*vi].y));
	}
}

void get_comp_geom(GGraph& lg, std::vector<int> components, V_g vertex, 
	mpoint_t &comp_geom) {
	int comp_id = components[vertex];
	for (int i = 0; i < components.size(); ++i) {
		if (components[i] == comp_id)
			bg::append(comp_geom, point_t(lg[i].x, lg[i].y));
	}
}

void get_square_bbox(bg::model::box<point_t>& bbox) {
	double min_x = bg::get<bg::min_corner, 0>(bbox);
    double min_y = bg::get<bg::min_corner, 1>(bbox);
    double max_x = bbox.max_corner().get<0>();
    double max_y = bbox.max_corner().get<1>();
	double l = max_x-min_x;
	double w = max_y-min_y;
	if (l == w) {
		return;
	}
	if (l > w) {
		max_y += (l-w)/2;
		min_y -= (l-w)/2;
	}
	else {
		max_x += (w-l)/2;
		min_x -= (w-l)/2;	
	}
	bbox.max_corner().set<0>(max_x);
    bbox.max_corner().set<1>(max_y);
    bbox.min_corner().set<0>(min_x);
    bbox.min_corner().set<1>(min_y);
}


void get_bbox(mpoint_t mp1, mpoint_t mp2, bg::model::box<point_t>& bbox) {
	mpoint_t total_comp;
	bg::append(total_comp, mp1);
	bg::append(total_comp, mp2);
	//bg::union_(mp1, mp2, total_comp);
	bg::envelope(total_comp, bbox);
	get_square_bbox(bbox);
}

void get_bbox(GGraph& lg, std::vector<int> components, V_g source, V_g target, 
	bg::model::box<point_t>& bbox) {
	mpoint_t source_comp, target_comp, total_comp;
	int source_comp_id, target_comp_id;
	get_comp_geom(lg, components, source, source_comp);
	get_comp_geom(lg, components, target, target_comp);
	//get_bbox(source_comp, target_comp, bbox);
}

void double_square_bbox(bg::model::box<point_t>& bbox) {
	double min_x = bg::get<bg::min_corner, 0>(bbox);
    double min_y = bg::get<bg::min_corner, 1>(bbox);
    double max_x = bbox.max_corner().get<0>();
    double max_y = bbox.max_corner().get<1>();
    double l = max_x-min_x;
    bbox.max_corner().set<0>(max_x+l/2);
    bbox.max_corner().set<1>(max_y+l/2);
    bbox.min_corner().set<0>(min_x-l/2);
    bbox.min_corner().set<1>(min_y-l/2);
}

bool is_within_polygon(point_t p, polygon_t b_polygon) {
	return bg::intersects(p, b_polygon)
			|| bg::within(p, b_polygon);
}

void get_bounding_graph(GGraph& g, GGraph& bg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_b,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_b,
	bg::model::box<point_t> bbox) {
	#if 1
	polygon_t b_polygon;
	bg::convert(bbox, b_polygon);
	std::cout << "polygon geom: " 
	<< bg::dsv(b_polygon) << std::endl;
	V_i_g vi;
	IO_i_g in, in_end;
	EO_i_g out, out_end;
	long int edge_count = 0, source, target;
	std::pair<GGraph::edge_descriptor, bool> p;
	std::set<V_g> remaining_vertices;
	std::set<V_g>::iterator it;
	/* Adding vertices within the bbox */
	for (vi = boost::vertices(g).first;
	        vi != boost::vertices(g).second; ++vi) {
		std::cout << "id: " << g[*vi].id << ", x: "
		<< g[*vi].x << ", y: " << g[*vi].y << ", is_within: " 
		<< is_within_polygon(point_t(g[*vi].x, g[*vi].y), b_polygon) << std::endl;
		if (is_within_polygon(point_t(g[*vi].x, g[*vi].y), b_polygon)) {
			if (id_to_V_b.find(g[*vi].id) == id_to_V_b.end()) {
				id_to_V_b[g[*vi].id] = boost::add_vertex(bg);
				bg[id_to_V_b[g[*vi].id]].id = g[*vi].id;
				bg[id_to_V_b[g[*vi].id]].x = g[*vi].x;
				bg[id_to_V_b[g[*vi].id]].y = g[*vi].y;
				remaining_vertices.insert(*vi);
			}
		}
	}
	V_g vi_g;
	/* Adding edges connected to these vertices */
	for (it = remaining_vertices.begin(); it != remaining_vertices.end(); ++it) {
		vi_g = *it;
		
	    std::cout << g[vi_g].id << std::endl;
	    for (boost::tie(in, in_end) = in_edges(vi_g, g);
	            in != in_end; ++in) {
	    		source = g[*in].source;
	    		target = g[*in].target;
	        	
	        	std::cout << ' '
	            <<  "=("
	            << g[*in].source << ", "
	            << g[*in].target << ") = "
	            << g[*in].weight <<"\t";
	            
	            if (is_within_polygon(point_t(g[id_to_V[source]].x, g[id_to_V[source]].y), b_polygon)
	            	&& is_within_polygon(point_t(g[id_to_V[target]].x, g[id_to_V[target]].y), b_polygon)) {
	            	add_edge_to_graph(bg, id_to_V_b, id_to_E_b,
	            		g[*in].id, source, target, g[*in].weight,
	            		g[id_to_V[source]].x, g[id_to_V[source]].y,
	            		g[id_to_V[target]].x, g[id_to_V[target]].y,
	            		g[*in].level);
	            }
	    }
	    for (boost::tie(out, out_end) = out_edges(vi_g, g);
	            out != out_end; ++out) {
	    		source = g[*out].source;
	    		target = g[*out].target;
	            
	            if (is_within_polygon(point_t(g[id_to_V[source]].x, g[id_to_V[source]].y), b_polygon)
	            	&& is_within_polygon(point_t(g[id_to_V[target]].x, g[id_to_V[target]].y), b_polygon)
	            	&& remaining_vertices.find(id_to_V_b[source]) != remaining_vertices.end()
	            	&& remaining_vertices.find(id_to_V_b[target]) == remaining_vertices.end()) {
	            	add_edge_to_graph(bg, id_to_V_b, id_to_E_b,
	            		g[*out].id, source, target, g[*out].weight,
	            		g[id_to_V[source]].x, g[id_to_V[source]].y,
	            		g[id_to_V[target]].x, g[id_to_V[target]].y,
	            		g[*out].level);
	            }
	    }
	    std::cout << std::endl;
	}
	#endif

}
