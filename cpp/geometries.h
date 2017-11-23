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
typedef boost::graph_traits < GGraph >::in_edge_iterator IO_i_g;




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

#if 0
/* 
Input:
comp_1 : component id
comp_geometries: A vector of geometries of components
comp_geometry[comp_1]: Geometry of comp_1
Function:
This function calculates the nearest component to comp_1
using the geometries of components
Output:
Id of the nearest component to comp_1
*/
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
#endif

int get_closest_comp(int comp_1, std::map<long int, mpoint_t>& comp_geometries) {
	//std::cout << "comp_geometries_size: " << comp_geometries.size()  << std::endl;
	std::map<long int, mpoint_t>::iterator it;
	mpoint_t comp_geom = comp_geometries[comp_1];
	long int nearest_comp = -1, temp_comp;
	int count = 1;
	double min_dist = std::numeric_limits<double>::max(), temp_dist;
	for (it = comp_geometries.begin(); it != comp_geometries.end(); ++it) {
		//std::cout << "count: " << count++  << std::endl;
		temp_comp = it->first;
		//std::cout << "temp_comp: "<< temp_comp  << std::endl;
		temp_dist = bg::distance(comp_geometries[comp_1], 
			comp_geometries[temp_comp]);
		//std::cout << "temp_dist: "<< temp_dist  << std::endl;
		if (temp_comp != comp_1 && (temp_dist - min_dist)<= 0.000000000001) {
			//std::cout << "yay" << std::endl;
			nearest_comp = temp_comp;
			//std::cout << "temp_comp: " << temp_comp << std::endl;
			//std::cout << "temp_comp_geom: " << bg::dsv(comp_geometries[temp_comp]) << std::endl;
			//std::cout << "comp1_geom: " << bg::dsv(comp_geometries[comp_1]) << std::endl;
			min_dist = temp_dist;

			//std::cout << "min_dist: " << min_dist << std::endl;
	}
}
//std::cout << "done with this" << std::endl;
return nearest_comp;
}

#if 0
/*
Input
g : Graph
components: components[i] is the id of component containing vertex i
Function:
Calculate the multi point geometries of all components from the point 
geometry of points in them
Output:
A vector of multi_point geometries

*/
void get_comp_geom(GGraph& g, std::vector<int> components, std::vector<mpoint_t>& comp_geometries) {
	V_i_g vi;
	comp_geometries.resize(components.size());
	for (vi = boost::vertices(g).first;
		vi != boost::vertices(g).second; ++vi) {
		bg::append(comp_geometries[components[*vi]], point_t(g[*vi].x, g[*vi].y));
}
}
#endif



void get_comp_geom(GGraph& g, std::vector<long int> components, std::map<long int, mpoint_t>& comp_geometries) {
	V_i_g vi;
	//comp_geometries.resize(components.size());
	for (vi = boost::vertices(g).first;
		vi != boost::vertices(g).second; ++vi) {
		//std::cout << "id: " << g[*vi].id << ", component: " << components[*vi] << std::endl;
		if (comp_geometries.find(components[*vi]) == comp_geometries.end()) {
			comp_geometries[components[*vi]] = mpoint_t(); 
		}
		bg::append(comp_geometries[components[*vi]], point_t(g[*vi].x, g[*vi].y));
}
}

#if 0
void get_comp_geom(GGraph& lg, std::vector<int> components, V_g vertex, 
	mpoint_t &comp_geom) {
	int comp_id = components[vertex];
	for (int i = 0; i < components.size(); ++i) {
		if (components[i] == comp_id)
			bg::append(comp_geom, point_t(lg[i].x, lg[i].y));
	}
}
#endif


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
	//bg::union_(mp1, mp2, total_comp);
	bg::envelope(total_comp, bbox);
	get_square_bbox(bbox);
}

#if 0
void get_bbox(GGraph& lg, std::vector<int> components, V_g source, V_g target, 
	bg::model::box<point_t>& bbox) {
	mpoint_t source_comp, target_comp, total_comp;
	int source_comp_id, target_comp_id;
	get_comp_geom(lg, components, source, source_comp);
	get_comp_geom(lg, components, target, target_comp);
	//get_bbox(source_comp, target_comp, bbox);
}
#endif


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
INput: source id and target id
Output: Edge joining source and target
*/
EdgeG get_edge(GGraph &g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
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


/*
Adds an edge to graph g and updates its maps accordingly
*/

void add_edge_to_graph(GGraph& g, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	long int eid,long int source,long int target, double weight,
	double x_1, double y_1,
	double x_2, double y_2, int level) {

	EdgeG edge = get_edge(g, id_to_V, id_to_E,
		source, target);
	if (edge.source != -1) {
		return;
	}


	std::pair<GGraph::edge_descriptor, bool> p;
	if (id_to_V.find(source) == id_to_V.end()) {
		//std::cout << "Adding vertex " <<  source << std::endl;
		//std::cout << "x: " << x_1 << ", y: " << y_1 << std::endl;
		id_to_V[source] = boost::add_vertex(g);
		g[id_to_V[source]].id = source;
		g[id_to_V[source]].x = x_1;
		g[id_to_V[source]].y = y_1;
	}
	if (id_to_V.find(target) == id_to_V.end()) {
		//std::cout << "Adding vertex " <<  target << std::endl;
		id_to_V[target] = boost::add_vertex(g);
		g[id_to_V[target]].id = target;
		g[id_to_V[target]].x = x_2;
		g[id_to_V[target]].y = y_2;

		//std::cout << "x: " << g[id_to_V[target]].x << ", y: " << g[id_to_V[target]].y << std::endl;
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

	//std::cout << "added :)" << std::endl;

}

/*
Constructs a graph with edges and vertices within a bounding box
*/
void get_bounding_graph(GGraph& g, GGraph& bg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_b,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_b,
	bg::model::box<point_t> bbox) {
	#if 1
	polygon_t b_polygon;
	bg::convert(bbox, b_polygon);
	//std::cout << "polygon geom: " 
	//<< bg::dsv(b_polygon) << std::endl;
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
		//std::cout << "id: " << g[*vi].id << ", x: "
	//<< g[*vi].x << ", y: " << g[*vi].y << ", is_within: " 
	//<< is_within_polygon(point_t(g[*vi].x, g[*vi].y), b_polygon) << std::endl;
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

	//std::cout << g[vi_g].id << std::endl;
	for (boost::tie(in, in_end) = in_edges(vi_g, g);
		in != in_end; ++in) {
		source = g[*in].source;
	target = g[*in].target;
	/*
	std::cout << ' '
	<<  "=("
	<< g[*in].source << ", "
	<< g[*in].target << ") = "
	<< g[*in].weight <<"\t";
	*/
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
//std::cout << std::endl;
}
	#endif

}

/*
Adds a path at a level to make the skeleton at that level strongly connected
Input: 
g: Original graph
lg: Graph at that level
path: A sequence of vertex ids of the path
level: Level of the graph
*/
void add_path_at_level(GGraph& g, 
	GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l,
	std::vector<long int>& path, int level) {

	//std::cout << "Adding path at level..." << std::endl;
	long int source, target;
	EdgeG edge;
	for (int i = 0; i < path.size()-1; ++i) {
		edge = get_edge(g, id_to_V, id_to_E,
		path[i], path[i+1]);
		//std::cout << "source: " << edge.source << ", target" << edge.target << std::endl;
		add_edge_to_graph(lg, id_to_V_l, id_to_E_l, edge.id,
		edge.source, edge.target, edge.weight, 
		g[id_to_V[edge.source]].x, g[id_to_V[edge.source]].y,
		g[id_to_V[edge.target]].x, g[id_to_V[edge.target]].y,
		level
		);
	}
}



void add_path_edges_to_result(GGraph& g, 
	GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l,
	std::vector<long int>& path, 
	std::vector<PromotedEdge>& promoted_edges, int level) {
	//std::cout << "Adding path edges to result..." << std::endl;
	long int source, target;
	EdgeG edge;
	PromotedEdge p_edge;
	for (int i = 0; i < path.size(); ++i) {
		edge = get_edge(lg, id_to_V_l, id_to_E_l,
			path[i], path[i+1]);
		if (edge.source == -1) {
			edge = get_edge(g, id_to_V, id_to_E,
				path[i], path[i+1]);
			if (edge.source != edge.target) {
				p_edge.id = edge.id;
				p_edge.source = edge.source;
				p_edge.target = edge.target;
				p_edge.level = level;
				promoted_edges.push_back(p_edge);
			}	
	}
}
}




void update_graph_with_path(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l,
	std::map<long int, std::set<long int> >& comp_vertices,
	std::map<long int, mpoint_t>& comp_geometries,
	std::vector<PromotedEdge>& promoted_edges,
	long int s_id, long int t_id,
	bg::model::box<point_t>& bbox, int level) {

	//std::cout << "Fetching bounding graph" << std::endl;
	GGraph bg;
	std::map<long int, GGraph::vertex_descriptor> id_to_V_b;
	std::map<long int, GGraph::edge_descriptor> id_to_E_b;

	get_bounding_graph(g, bg, 
		id_to_V, id_to_E, 
		id_to_V_b, id_to_E_b,
		bbox);
	// Fetch their vertex descriptors
	std::vector<long int> path;
	V_g bg_s = id_to_V_b[s_id];
	V_g bg_t = id_to_V_b[t_id];

	// Double the bounding box until path is found
	while (!get_astar_path(bg, bg_s, bg_t, path)) {
		//print_geom_graph(bg);
		double_square_bbox(bbox);
		get_bounding_graph(g, bg, 
			id_to_V, id_to_E, 
			id_to_V_b, id_to_E_b,
			bbox);
	}
	/*
	std::cout << "Path " << std::endl;
	for (int j = 0; j < path.size(); ++j) {
		std::cout << path[j] << ", ";
	}
	std::cout << std::endl;
	*/
	

	// Add the edges of path to the result
	add_path_edges_to_result(g, lg,
		id_to_V, id_to_E,
		id_to_V_l, id_to_E_l,
		path, promoted_edges, level);

	//std::cout << "Path size: " << path.size() << std::endl;


	#if 1	
	// Add the geometries of path vertices to component 0
	for (int i = 0; i < path.size(); ++i) {
		if (id_to_V_l.find(path[i]) == id_to_V_l.end()) {
			
			bg::append(comp_geometries[0], point_t(g[id_to_V[path[i]]].x, 
				g[id_to_V[path[i]]].y));
		}
	}
	#endif


	// Add path at this level
	add_path_at_level(g, lg, id_to_V, id_to_E,
		id_to_V_l, id_to_E_l,
		path, level);


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
			level = std::atof(result[8].c_str());
			add_edge_to_graph(g, id_to_V, id_to_E, 
				eid, source, target, cost, 
				x_1, y_1, x_2, y_2, level);
			
		}

	}
	return 1;
	
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
Prints the component id and its geometry
*/

void print_comp_geometries(std::vector<mpoint_t> comp_geometries) {
	for (int i = 0; i < comp_geometries.size(); ++i) {
		std::cout << "comp: " << i << ", geom: " << bg::dsv(comp_geometries[i]) << std::endl;
	}
}

void print_comp_geometries(std::map<long int, mpoint_t>& comp_geometries) {
	std::map<long int, mpoint_t>::iterator it;
	for (it = comp_geometries.begin(); it != comp_geometries.end(); ++it) {
		std::cout << "comp: " << it->first << ", geom: " << bg::dsv(comp_geometries[it->first]) << std::endl;
	}
}

/*
Adds all the edges of a level from g to lg
*/
void get_graph_at_level(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
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
	    //std::cout << g[*vi].id << ": " << " out_edges_of(" << g[(*vi)].id << "):";
		for (boost::tie(out, out_end) = out_edges(*vi, g);
			out != out_end; ++out) {

			source = g[*out].source;
			target = g[*out].target;
			edge = get_edge(lg, id_to_V_l, id_to_E_l,
				source, target);
			if (edge.source == edge.target) {

				if (g[*out].level <= level) {
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

*/
V_g get_vertex_from_comp(std::vector<long int> components, long int comp_id) {
	for (int i = 0; i < components.size(); ++i) {
		if (components[i] == comp_id) {
			return i;
		}
	}
	return -1;
}






void _strong_connect_components(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l,
	std::map<long int, std::set<long int> >& comp_vertices,
	std::map<long int, mpoint_t >& comp_geometries,
	std::vector<PromotedEdge>& promoted_edges, int level) {

	int nearest_comp;
	bg::model::box<point_t> bbox;
	long int s_id, t_id;
	while(comp_geometries.size() != 1) {
		/*
		std::cout << "Level Graph before" << std::endl;
		print_geom_graph(lg);
		*/
		/*
		std::cout << "components: " << std::endl;
		print_vertex_components(lg, components);
		*/
		/*
		std::cout << "geometries: " << std::endl;
		print_comp_geometries(comp_geometries);
		*/
		std::cout << "num components: " << comp_geometries.size() << std::endl;
		/*
		std::cout << "Comp geometries before" << std::endl;
		print_comp_geometries(comp_geometries);
		*/
		//std::cout << "Fetching nearest_comp" << std::endl;
		nearest_comp = get_closest_comp(0, comp_geometries);

		
		//std::cout << "nearest component to comp 0" << std::endl;
		//std::cout << "id: " << nearest_comp << std::endl;
		//std::cout << "geom: " << bg::dsv(comp_geometries[nearest_comp]) << std::endl;
		if (nearest_comp == -1) {
			std::cout << "Error finding nearest_comp" << std::endl;
			return ;
		}
		

		//Obtain the square bbox enclosing component 0 and its nearest comp
		//std::cout << "Fetching bbox" << std::endl;
		get_bbox(comp_geometries[0], 
			comp_geometries[nearest_comp],
			bbox);


		//std::cout << "Pick source and target" << std::endl;
		// Pick the first vertices of both the components as source and target
		s_id = *(comp_vertices[0].begin());
		t_id = *(comp_vertices[nearest_comp].begin());

		//std::cout << "s: " << s_id << ", t: " << t_id << std::endl;
	
		//std::cout << "Updating based on path" << std::endl;
		// Adds path from source to target
		update_graph_with_path(g, lg,
			id_to_V, id_to_E,
			id_to_V_l, id_to_E_l,
			comp_vertices, comp_geometries,
			promoted_edges,
			s_id, t_id, bbox, level);

		
		// Adds path from target to source
		update_graph_with_path(g, lg,	
			id_to_V, id_to_E,
			id_to_V_l, id_to_E_l,
			comp_vertices, comp_geometries,
			promoted_edges,
			t_id, s_id, bbox, level);


		#if 1
		// Add geometry of nearest comp to component 0
		//std::cout << "Updating comp 0 geom" << std::endl;
		bg::append(comp_geometries[0], comp_geometries[nearest_comp]);

		// Erase the geometry of nearest comp
		//std::cout << "Erasing nearest_comp geom" << std::endl;
		comp_geometries.erase(nearest_comp);
		//comp_geometries[nearest_comp] = empty_mpoint;
		
		//std::cout << "Updating comp 0 vertices" << std::endl;
		// Add the component vertices of nearest comp to comp 0
		comp_vertices[0].insert(comp_vertices[nearest_comp].begin(),
			comp_vertices[nearest_comp].end());
		comp_vertices.erase(nearest_comp);


	}


	#endif
}


void get_comp_vertices(GGraph& g, 
	std::vector<long int> components,
	std::map<long int, std::set<long int> >& comp_vertices) {
	for (int i = 0; i < components.size(); ++i) {
		comp_vertices[components[i]].insert(g[i].id);
	}
}

#if 1
void strong_connect_components(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l, 
	std::vector<PromotedEdge>& promoted_edges, int level) {


	// Obtain the graph at a particular level
	get_graph_at_level(g, lg, id_to_V, id_to_E, id_to_V_l, id_to_E_l, level);
	/*
	std::cout << "Level Graph before" << std::endl;
	print_geom_graph(lg);
	*/
	// Compute its components
	std::vector<long int> components(num_vertices(lg));
	int num_comps = boost::strong_components(lg, 
		boost::make_iterator_property_map(components.begin(), get(boost::vertex_index, lg)));

	if (num_comps == 1) {
		return;
	}


	std::map<long int, mpoint_t> comp_geometries;
	std::map<long int, std::set<long int> > comp_vertices;

	get_comp_vertices(lg, components, comp_vertices);

	/*
	std::cout << "components: " << std::endl;
	print_vertex_components(lg, components);
	*/
	// Compute geometry of components
	get_comp_geom(lg, components, comp_geometries);
	/*
	std::cout << "geometries: " << std::endl;
	print_comp_geometries(comp_geometries);
	*/
	// Connect them
	_strong_connect_components(g, lg, 
		id_to_V, id_to_E, 
		id_to_V_l, id_to_E_l,
		comp_vertices, comp_geometries, 
		promoted_edges, level);
}
#endif


void strong_connect_components_levels(GGraph& g, GGraph& lg, 
	std::map<long int, GGraph::vertex_descriptor>& id_to_V,
	std::map<long int, GGraph::edge_descriptor>& id_to_E,
	std::map<long int, GGraph::vertex_descriptor>& id_to_V_l,
	std::map<long int, GGraph::edge_descriptor>& id_to_E_l, 
	std::vector<PromotedEdge>& promoted_edges, int max_level) {

	for (int i = 1; i <= max_level; ++i) {
		std::cout << "Level: " << i << std::endl;
		strong_connect_components(g, lg, 
			id_to_V, id_to_E, 
			id_to_V_l, id_to_E_l, 
			promoted_edges, i);
		std::vector<long int> components(num_vertices(lg));
		int num_comps = boost::strong_components(lg, 
			boost::make_iterator_property_map(components.begin(), get(boost::vertex_index, lg)));
		assert(num_comps == 1);
	}
}
