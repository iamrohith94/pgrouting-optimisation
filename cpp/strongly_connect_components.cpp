
#include <iostream>
#include <set>
#include <boost/config.hpp>

#include "geometries.h"


int main(int argc, char const *argv[])
{

	std::string file_name(argv[1]);
	int num_levels, nearest_comp;

	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;


	std::map<long int, GGraph::vertex_descriptor> id_to_V;
	std::map<long int, GGraph::edge_descriptor> id_to_E;
	GGraph g;
	// Constructing the whole graph
	int flag = construct_graph_with_geometry(file_name, ',', g, id_to_V, id_to_E);
	if (flag != 1) {
		std::cout << "Error in graph construction" << std::endl;
		return 0;
	}

	//std::cout << "Original Graph: " << std::endl;
	//print_geom_graph(g);
	std::cout << "Num of edges: " << num_edges(g) << std::endl;
	std::cout << "Num of vertices: " << num_vertices(g) << std::endl;
	GGraph lg;
	std::map<long int, GGraph::vertex_descriptor> id_to_V_l;
	std::map<long int, GGraph::edge_descriptor> id_to_E_l;
	std::vector<Connection> Connection;
	strong_connect_components_levels(g, lg, 
			id_to_V, 
			//id_to_E,
			id_to_V_l, id_to_E_l,
			Connection, num_levels);

	//std::cout << "Promoted Edges: " << std::endl;
	for (int i = 0; i < Connection.size(); ++i) {
		/*
		std::cout << "id: " << promoted_edges[i].id
		<< ", source: " << promoted_edges[i].source
		<< ", target: " << promoted_edges[i].target
		<< ", level: " << promoted_edges[i].level
		<< std::endl;
		*/
		std::cout 
		<< Connection[i].source
		<< ", " << Connection[i].target
		<< ", " << Connection[i].level
		<< std::endl;
	}
	#if 0
	get_graph_at_level(g, lg, id_to_V, id_to_E, id_to_V_l, id_to_E_l, 1);
	for (int i = 1; i <= num_levels; ++i) {
		std::cout << "Level: " << i << std::endl;
		std::cout << "Graph: " << std::endl;
		print_geom_graph(lg);
		std::vector<int> components(num_vertices(lg));
		int num = boost::strong_components(lg, 
			boost::make_iterator_property_map(components.begin(), get(boost::vertex_index, lg)));
		std::cout << "components: " << std::endl;
		print_vertex_components(lg, components);
		std::vector<mpoint_t> comp_geometries;
		get_comp_geom(lg, components, comp_geometries);
		std::cout << "geometries: " << std::endl;
		print_comp_geometries(comp_geometries);
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

		V_g s = get_vertex_from_comp(components, 0);
		V_g t = get_vertex_from_comp(components, nearest_comp);
		long int s_id = lg[s].id;
		long int t_id = lg[t].id;

		std::vector<long int> path;
		V_g bg_s = id_to_V_b[s_id];
		V_g bg_t = id_to_V_b[t_id];

		std::cout << "source: " << bg[bg_s].id << ", target: " << bg[bg_t].id << std::endl;
		if (!get_astar_path(bg, bg_s, bg_t, path)) {
			std::cout << "No path !!" << std::endl;
			return 0;
		}
		std::cout << "Path " << std::endl;
		for (int j = 0; j < path.size(); ++j) {
			std::cout << path[j] << ", ";
		}
		std::cout << std::endl;

		add_path_at_level(g, id_to_V, id_to_E,
			lg, id_to_V_l, id_to_E_l,
			path, i);

		print_geom_graph(lg);
		/*
		std::cout << "bbox geometry after doubling" << std::endl;
		double_square_bbox(bbox);
		std::cout << bg::dsv(bbox) << std::endl;

		get_bounding_graph(g, bg, 
			id_to_V, id_to_E, 
			id_to_V_b, id_to_E_b,
			bbox);
		print_geom_graph(bg);
		*/

	}
	#endif



	return 0;
}
