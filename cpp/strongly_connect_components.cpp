
#include <iostream>
#include <set>
#include <boost/graph/strong_components.hpp>
#include <boost/config.hpp>

#include "geometries.h"


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
	print_geom_graph(g);

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