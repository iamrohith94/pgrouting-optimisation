#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <boost/graph/strong_components.hpp>
#include <fstream>
#include "astar.h"

namespace bg = boost::geometry;

typedef bg::cs::cartesian geometry_type;  
typedef bg::model::point<double, 2, geometry_type> point_t;
typedef bg::model::multi_point<point_t> mpoint_t;
typedef bg::model::polygon<point_t> polygon_t;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, VertexG, EdgeG> GGraph;


typedef  boost::graph_traits < GGraph >::vertex_descriptor V_g;
typedef  boost::graph_traits < GGraph >::edge_descriptor E_g;
typedef  boost::graph_traits < GGraph >::vertex_iterator V_i_g;
typedef  boost::graph_traits < GGraph >::edge_iterator E_i_g;
typedef boost::graph_traits < GGraph >::out_edge_iterator EO_i_g;
typedef boost::graph_traits < GGraph >::in_edge_iterator EI_i_g;
typedef std::map<long int, std::set<long int> > ComponentVertices;



/* Prints the graph with geometrical attributes */
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


/*
Prints the vertex id and component id containing the vertex
*/
void print_vertex_components(GGraph g, std::vector<long int> components) {
	V_i_g vi;
	for (vi = boost::vertices(g).first;
		vi != boost::vertices(g).second; ++vi) {
		std::cout << "id: " << g[*vi].id << ", comp: " << components[*vi] << std::endl;
}
}

/*
 * Prints the component id and its vertices
 * */
void print_comp_vertices(std::map<long int, std::set<long int> >& comp_vertices) {
        std::map<long int, std::set<long int> >::iterator it;
	std::set<long int>::iterator it_v;
        for (it = comp_vertices.begin(); it != comp_vertices.end(); ++it) {
                std::cout << "comp: " << it->first << std::endl;
		for (it_v = it->second.begin(); it_v != it->second.end(); ++it_v) {
			std::cout << *it_v << ", "  << std::endl;
		}
		std::cout << std::endl;
        }
}

/*
Input: A bounding box
Output: Converts the given bounding box into a square bounding box
by expansion
*/
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

/*
Input : two multi_point geometries
Output: A square bounding box enclosing both the multipoint geometries 
*/

void get_bbox(mpoint_t mp1, mpoint_t mp2, bg::model::box<point_t>& bbox) {
	mpoint_t total_comp;
	bg::append(total_comp, mp1);
	bg::append(total_comp, mp2);
	bg::envelope(total_comp, bbox);
	get_square_bbox(bbox);
}

void get_bbox(point_t p1, point_t p2, bg::model::box<point_t>& bbox) {
	mpoint_t total_comp;
	bg::append(total_comp, p1);
	bg::append(total_comp, p2);
	bg::envelope(total_comp, bbox);
	get_square_bbox(bbox);
}



/*
Input : A sqaure bounding box
Output: Converts the given bounding box into a bounding box
of double the size 
*/
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

/*
Input: A point and a polygon
Output: Returns true if
		1. Point inside polygon
		2. Point on the polygon(boundary)

*/
bool is_within_polygon(point_t p, polygon_t b_polygon) {
	return bg::intersects(p, b_polygon)
	|| bg::within(p, b_polygon);
}

/*
INput: source id and target id
Output: Edge joining source and target
*/
EdgeG get_edge(GGraph &g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	//std::map<long int, GGraph::edge_descriptor>& id_to_E,
	long int source, long int target) {
	EO_i_g out, out_end;
	EdgeG edge;
	edge.source = edge.target = -1;
	if (id_to_V.find(source) == id_to_V.end() || id_to_V.find(target) == id_to_V.end()) {
		return edge;
	}
	else {
		for (boost::tie(out, out_end) = out_edges(id_to_V[source], g);
			out != out_end; ++out) {
			if (g[*out].target == target) {
				edge = g[*out];
				return edge;
			}
		}
	}
	
	return edge;
}

bool get_edge(GGraph &g,
        std::map<long int, GGraph::vertex_descriptor>& id_to_V,
        //std::map<long int, GGraph::edge_descriptor>& id_to_E,
        long int eid, long int source, long int target, double cost) {
	EO_i_g out, out_end;
	if (id_to_V.find(source) == id_to_V.end() || id_to_V.find(target) == id_to_V.end()) {
                return false;
        }
        else {
                for (boost::tie(out, out_end) = out_edges(id_to_V[source], g);
                        out != out_end; ++out) {
                        if (g[*out].target == target && g[*out].id == eid && g[*out].weight == cost) {
                                return true;
                        }
                }
        }

        return false;
}


/*
Input:
bg : graph
source : vertex descriptor of source
target : vertex descriptor of target
Output
Returns true if path is found
path: contains the paths(seq of vertices) from source to target if path is found
*/
bool get_astar_path(GGraph& bg, V_g source, V_g target, 
	std::vector<long int>& path) {
	Pgr_astar<GGraph> Astar;
	if(Astar.do_astar(bg, source, target)) {
		Astar.get_path(bg, source, target, path);
		return true;
	}
	return false;
}


/*
Input:
bg : graph
source : vertex descriptor of source
target : vertex descriptor of target
Output
Returns true if path is found
path: contains the paths(seq of vertices) from source to target if path is found
*/
bool get_astar_edges(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::vector<long int>& path, 
	std::vector<PromotedEdge>& promoted_edges,
	int level) {
	long int source, target;
		EdgeG edge;
		PromotedEdge p_edge;
		for (int i = 0; i < path.size()-1; ++i) {
			edge = get_edge(g, id_to_V, 
				path[i], path[i+1]);
			assert(edge.source != -1);
			p_edge.id = edge.id;
			p_edge.source = edge.source;
			p_edge.target = edge.target;
			p_edge.level = level;
			promoted_edges.push_back(p_edge);
	}
	assert(promoted_edges[promoted_edges.size()-1].target == path[path.size()-1]);
}


/*
Adds an edge to graph g and updates its maps accordingly
*/

void add_vertex_to_graph(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	long int vid,
	double x, double y) {

	if (id_to_V.find(vid) == id_to_V.end()) {
		//std::cout << "Adding vertex " <<  source << std::endl;
		//std::cout << "x: " << x_1 << ", y: " << y_1 << std::endl;
		id_to_V[vid] = boost::add_vertex(g);
		g[id_to_V[vid]].id = vid;
		g[id_to_V[vid]].x = x;
		g[id_to_V[vid]].y = y;
	}
}


/*
Adds an edge to graph g and updates its maps accordingly
*/

void add_edge_to_graph(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	long int eid,long int source,long int target, double weight,
	double x_1, double y_1,
	double x_2, double y_2, int level) {

	if(get_edge(g, id_to_V, 
		eid,
		source, target, weight))
		return;

	std::pair<GGraph::edge_descriptor, bool> p;
	add_vertex_to_graph(g, id_to_V, source, x_1, y_1);
	add_vertex_to_graph(g, id_to_V, target, x_2, y_2);
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



/*
Constructs a graph from a csv file with the following format
id: Id of the edge
source: source id of the edge
target: target id of the edge
cost: edge weight
x_1: x coord of source
y_1: y coord of source
x_2: x coord of target
y_2: y coord of target
*/
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
			level = std::atoi(result[8].c_str());
			add_edge_to_graph(g, id_to_V, id_to_E, 
				eid, source, target, cost, 
				x_1, y_1, x_2, y_2, level);
			
		}

	}
	return 1;
	
}

/*
Adds all the edges of a level from g to lg
*/
void get_graph_at_level(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l, int level) {

	V_i_g vi;
	EO_i_g out, out_end;
	long int source, target;
	EdgeG edge;


	std::pair<GGraph::edge_descriptor, bool> p;
	/* Adding edges at a given level */
	for (vi = boost::vertices(g).first;
		vi != boost::vertices(g).second; ++vi) {
		for (boost::tie(out, out_end) = out_edges(*vi, g);
			out != out_end; ++out) {

			source = g[*out].source;
			target = g[*out].target;
			edge = get_edge(lg, id_to_V_l, 
				source, target);
			if (edge.source == -1) {

				if (g[*out].level == level) {
					add_edge_to_graph(lg, id_to_V_l, id_to_E_l,
						g[*out].id, source, target, g[*out].weight,
						g[id_to_V[source]].x, g[id_to_V[source]].y,
						g[id_to_V[target]].x, g[id_to_V[target]].y,
						g[*out].level);
				}
			}
		}
	}
}

/*
Constructs a graph with edges and vertices within a bounding box
*/
void get_bounding_graph(GGraph& g, GGraph& bg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	//std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_b,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_b,
	bg::model::box<point_t> bbox) {
	polygon_t b_polygon;
	bg::convert(bbox, b_polygon);
	V_i_g vi;
	EI_i_g in, in_end;
	EO_i_g out, out_end;
	long int edge_count = 0, source, target;
	std::pair<GGraph::edge_descriptor, bool> p;
	std::set<V_g> remaining_vertices;
	std::set<V_g>::iterator it;
	/* Adding vertices within the bbox */
	for (vi = boost::vertices(g).first;
		vi != boost::vertices(g).second; ++vi) {
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

	for (boost::tie(in, in_end) = in_edges(vi_g, g);
		in != in_end; ++in) {
		source = g[*in].source;
	target = g[*in].target;
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
	&& remaining_vertices.find(id_to_V[source]) != remaining_vertices.end()
	&& remaining_vertices.find(id_to_V[target]) == remaining_vertices.end()) {
	add_edge_to_graph(bg, id_to_V_b, id_to_E_b,
		g[*out].id, source, target, g[*out].weight,
		g[id_to_V[source]].x, g[id_to_V[source]].y,
		g[id_to_V[target]].x, g[id_to_V[target]].y,
		g[*out].level);
}
}
}

}


void get_connecting_edges(GGraph& g,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	long int s_id, long int t_id, 
	std::vector<PromotedEdge>& promoted_edges, 
	int level) {


	bg::model::box<point_t> bbox;
	
	//Generating bbox 
	
	get_bbox(point_t(g[id_to_V[s_id]].x, g[id_to_V[s_id]].y), 
		point_t(g[id_to_V[t_id]].x, g[id_to_V[t_id]].y),
		bbox);
	GGraph bg;
	std::map<long int, GGraph::vertex_descriptor> id_to_V_b;
	std::map<long int, GGraph::edge_descriptor> id_to_E_b;

	//Generating bounding graph
	get_bounding_graph(g, bg, 
		id_to_V, 
		//id_to_E, 
		id_to_V_b, id_to_E_b,
		bbox);

	//Doubling initially
	//double_square_bbox(bbox);

	// Fetch their vertex descriptors
	std::vector<long int> path;
	V_g bg_s = id_to_V_b[s_id];
	V_g bg_t = id_to_V_b[t_id];

	// Double the bounding box until path is found
	while (!get_astar_path(bg, bg_s, bg_t, path)) {
		path.clear();
		//print_geom_graph(bg);
		double_square_bbox(bbox);
		get_bounding_graph(g, bg, 
			id_to_V, 
			//id_to_E, 
			id_to_V_b, id_to_E_b,
			bbox);
	}
	
	get_astar_edges(bg, id_to_V_b, path, promoted_edges, level);
}



void makeStronglyConnected(GGraph &g,
		std::map<long int, GGraph::vertex_descriptor>& id_to_V,
        ComponentVertices &component_vertices, 
        std::vector<Connection>& connections,
        int level) {
	size_t totalNodes = num_vertices(g);
	std::vector< long int > components(totalNodes);
	//Finding connected components
    int num_comps;
    Connection temp;

    //print_geom_graph(g);
    num_comps =  boost::strong_components(g,
        boost::make_iterator_property_map(components.begin(),
                                          get(boost::vertex_index,
                                              g)));
    //std::cout << "Initial connected components: " << num_comps << std::endl;
    //std::cout << "Initial num vertices: " << totalNodes << std::endl;
    
    if (num_comps == 1) {
        return ;
    }
    ComponentVertices temp_component_vertices;
    //std::vector< VertexProperties > DAG_vertices(num_comps);

    /* Storing the vids of the components*/  
    for (size_t i = 0; i < totalNodes; i++) {
        temp_component_vertices[components[i]].
        insert(component_vertices[i].begin(), component_vertices[i].end());
    }
    

    GGraph DAG;
    std::map<long int, GGraph::vertex_descriptor> id_to_V_DAG;
    std::map<long int, GGraph::edge_descriptor> id_to_E_DAG;
    /* Adding the component ids to the DAG vertices */
    for (size_t i = 0; i < num_comps; i++) {
        add_vertex_to_graph(DAG, id_to_V_DAG, i, -1.00, -1.00);
    }

    //Add edges to DAG
    for (auto eit = boost::edges(g).first; 
                        eit != boost::edges(g).second; ++eit) {
        V_g s, t;
        s = id_to_V[g[*eit].source];
        t = id_to_V[g[*eit].target];
        if (components[s] != components[t]) {
            add_edge_to_graph(DAG, id_to_V_DAG, id_to_E_DAG, 
            	boost::num_edges(DAG), components[s], components[t], g[*eit].weight,
				g[s].x, g[s].y,
				g[t].x, g[t].y,
				level);
        }
    }
    long int curr_vid, from_comp_id, to_comp_id;
    Shortcut s;
    //Compute Connections
    for (auto vi = vertices(DAG).first;
                        vi != vertices(DAG).second; ++vi) {

    	curr_vid = DAG[*vi].id;
    	if (boost::in_degree(*vi, DAG) != 0 && boost::out_degree(*vi, DAG) != 0) {
            continue;
        }
        
        if (boost::in_degree(*vi, DAG) == 0 && boost::out_degree(*vi, DAG) == 0) {
            from_comp_id = curr_vid;
            to_comp_id = (curr_vid +1)%num_vertices(DAG); 
            //add edge to DAG
            s.cost = 1;
            s.reverse_cost = 1;
        }
        else if(boost::out_degree(*vi, DAG) == 0) {
            from_comp_id = curr_vid;
            EI_i_g eit, eit_end;
            boost::tie(eit, eit_end) = in_edges(*vi, DAG);
            to_comp_id = DAG[source(*eit, DAG)].id;
            //add edge to DAG
            s.cost = 1;
            s.reverse_cost = -1;
        }

        else {
            EO_i_g eit, eit_end;
            boost::tie(eit, eit_end) = out_edges(*vi, DAG);
            from_comp_id = DAG[target(*eit, DAG)].id;
            to_comp_id = curr_vid;
            //add edge to DAG
            s.cost = 1;
            s.reverse_cost = -1;
        }
        s.source = from_comp_id;
        s.target = to_comp_id;

        if (s.cost > 0) {
        
	        temp.source = *(temp_component_vertices[from_comp_id].begin());
	        temp.target = *(temp_component_vertices[to_comp_id].begin());
	        temp.level = level;
	        /*
	        std::cout << "s: " << temp.source << ", "
	        << "t: " << temp.target << ", "
	        <<  ", level: " << level << std::endl; 
            */
	        connections.push_back(temp);
	        add_edge_to_graph(DAG, id_to_V_DAG, id_to_E_DAG, 
	            	boost::num_edges(DAG), s.source, s.target, s.cost,
					-1.000, -1.000,
					-1.000, -1.000,
					level);
        }

        if (s.reverse_cost > 0) {
	        temp.source = *(temp_component_vertices[to_comp_id].begin());
	        temp.target = *(temp_component_vertices[from_comp_id].begin());
	        temp.level = level;
	        /*
	        std::cout << "s: " << temp.target << ", "
	        << "t: " << temp.source << ", "
	        <<  ", level: " << level << std::endl; 
	        */
	        connections.push_back(temp);

	        add_edge_to_graph(DAG, id_to_V_DAG, id_to_E_DAG, 
	            	boost::num_edges(DAG), s.target, s.source, s.reverse_cost,
					-1.000, -1.000,
					-1.000, -1.000,
					level);
        }
    }

    makeStronglyConnected(DAG, id_to_V_DAG, temp_component_vertices, connections, level);  
}


void get_connections_at_level(GGraph &g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
        std::vector<Connection>& connections,
        int level) {

	ComponentVertices comp_vertices;
	for (auto vi = vertices(g).first;
	                   vi != vertices(g).second; ++vi) {
	       comp_vertices[*vi].insert(g[*vi].id);
	}
	makeStronglyConnected(g, id_to_V, comp_vertices, connections, level);   
}



void get_connections_at_levels(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::vector<Connection>& connections, int max_level) {

	GGraph lg;
	std::map<long int, GGraph::vertex_descriptor> id_to_V_l;
	std::map<long int, GGraph::edge_descriptor> id_to_E_l;
	for (int i = 1; i <= max_level; ++i) {
		std::vector<Connection> temp_connections;
		//std::cout << "Level: " << i << std::endl;
		get_graph_at_level(g, lg, id_to_V,
			id_to_V_l, id_to_E_l, i);

		get_connections_at_level(lg, 
			id_to_V_l, 
			temp_connections, i);

		for (int j = 0; j < temp_connections.size(); ++j) {
			add_edge_to_graph(lg, id_to_V_l, id_to_E_l, 
				boost::num_edges(lg), temp_connections[j].source, 
				temp_connections[j].target, 1.00,
					-1.000, -1.000,
					-1.000, -1.000,
					i);
		}
		std::vector<long int> components(num_vertices(lg));
		int num_comps = boost::strong_components(lg, 
			boost::make_iterator_property_map(components.begin(), get(boost::vertex_index, lg)));
		assert(num_comps == 1);
		connections.insert( connections.end(), temp_connections.begin(), temp_connections.end() );
	}
}
