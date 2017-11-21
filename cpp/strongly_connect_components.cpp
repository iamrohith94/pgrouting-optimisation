#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/union.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/graph/connected_components.hpp>
#include <iostream>
#include <set>
#include <boost/graph/use_mpi.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/config.hpp>

#include "helper.h"
namespace bg = boost::geometry;


typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::multi_point<point_t> mpoint_t;
typedef bg::model::polygon<point_t> polygon_t;

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
	    		if (id_to_V_l.find(source) == id_to_V_l.end()) {
					id_to_V_l[source] = boost::add_vertex(lg);
					lg[id_to_V_l[source]].id = source;
					lg[id_to_V_l[source]].x = g[id_to_V[source]].x;
					lg[id_to_V_l[source]].y = g[id_to_V[source]].y;
				}
				if (id_to_V_l.find(target) == id_to_V_l.end()) {
					id_to_V_l[target] = boost::add_vertex(lg);
					lg[id_to_V_l[target]].id = target;
					lg[id_to_V_l[target]].x = g[id_to_V[target]].x;
					lg[id_to_V_l[target]].y = g[id_to_V[target]].y;
				}
				p = boost::add_edge(id_to_V_l[source], id_to_V_l[target], lg);
				lg[p.first].weight = g[*out].weight;
				lg[p.first].source = source;
				lg[p.first].target = target;
				lg[p.first].idx = id_to_E_l.size();
				id_to_E_l[lg[p.first].idx] = p.first;
				lg[p.first].id = g[*out].id;
				lg[p.first].level = g[*out].level;
	    	}
	    }
	}
}

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
	            	p = boost::add_edge(id_to_V_b[g[*in].source], id_to_V_b[g[*in].target], bg);
	            	bg[p.first].weight = g[*in].weight;
	            	bg[p.first].source = g[*in].source;
	            	bg[p.first].target = g[*in].target;
	            	bg[p.first].idx = id_to_E_b.size();
	            	id_to_E_b[bg[p.first].idx] = p.first;
	            	bg[p.first].id = g[*in].id;
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
	            	p = boost::add_edge(id_to_V_b[g[*out].source], id_to_V_b[g[*out].target], bg);
	            	bg[p.first].weight = g[*out].weight;
	            	bg[p.first].source = g[*out].source;
	            	bg[p.first].target = g[*out].target;
	            	bg[p.first].idx = id_to_E_b.size();
	            	id_to_E_b[bg[p.first].idx] = p.first;
	            	bg[p.first].id = g[*out].id;
	            }
	    }
	    std::cout << std::endl;
	}
	#endif

}

int main(int argc, char const *argv[])
{

	std::string file_name(argv[1]);
	int num_levels, nearest_comp;

	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 1;


	std::map<long int, GGraph::vertex_descriptor> id_to_V;
	std::map<long int, GGraph::edge_descriptor> id_to_E;
	GGraph g;
	// Constructing the whole graph
	int flag = construct_graph_with_geometry(file_name, ',', g, id_to_V, id_to_E);
	if (flag != 1) {
		std::cout << "Error in graph construction" << std::endl;
		return 0;
	}

	std::cout << "Original Graph: " << std::endl;
	print_graph(g);

	GGraph lg;
	std::map<long int, GGraph::vertex_descriptor> id_to_V_l;
	std::map<long int, GGraph::edge_descriptor> id_to_E_l;
	get_graph_at_level(g, lg, id_to_V, id_to_E, id_to_V_l, id_to_E_l, 1);
	for (int i = 1; i <= num_levels; ++i) {
		std::cout << "Level: " << i << std::endl;
		std::cout << "Graph: " << std::endl;
		print_geom_graph(lg);
		std::vector<int> components(num_vertices(lg));
		int num = boost::strong_components(lg, 
			boost::make_iterator_property_map(components.begin(), get(boost::vertex_index, lg)));
		std::cout << "components: " << std::endl;
		print_components(lg, components);
		std::vector<mpoint_t> comp_geometries;
		get_comp_geom(lg, components, comp_geometries);
		std::cout << "geometries: " << std::endl;
		print_geometries(comp_geometries);
		if (num == 1) {
			std::cout << "Only one component" << std::endl;
			return 0;
		}
		nearest_comp = get_closest_comp(0, comp_geometries);
		if (nearest_comp == -1) {
			std::cout << "Error finding nearest_comp" << std::endl;
			return 0;
		}
		std::cout << "nearest component to comp 0" << std::endl;
			
		std::cout << "id: " << nearest_comp << std::endl;
		std::cout << "geom: " << bg::dsv(comp_geometries[nearest_comp]) << std::endl;
		bg::model::box<point_t> bbox;
		get_bbox(comp_geometries[0], 
			comp_geometries[nearest_comp],
			bbox);
		std::cout << "bbox geometry" << std::endl;
		std::cout << bg::dsv(bbox) << std::endl;


		GGraph bg;
		std::map<long int, GGraph::vertex_descriptor> id_to_V_b;
		std::map<long int, GGraph::edge_descriptor> id_to_E_b;
		get_bounding_graph(g, bg, 
			id_to_V, id_to_E, 
			id_to_V_b, id_to_E_b,
			bbox);

		print_geom_graph(bg);



		std::cout << "bbox geometry after doubling" << std::endl;
		double_square_bbox(bbox);
		std::cout << bg::dsv(bbox) << std::endl;

		get_bounding_graph(g, bg, 
			id_to_V, id_to_E, 
			id_to_V_b, id_to_E_b,
			bbox);
		print_geom_graph(bg);

	}



	return 0;
}