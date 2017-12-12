#include <boost/mpi.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, comp_sql, residue_sql,
	skeletal_v_update_sql, non_skeletal_v_update_sql, cut_e_update_sql, 
	non_skeletal_e_update_sql, edges_sql, non_skeletal_v_sql, non_skeletal_vertices;
	edge_table = "cleaned_ways";
	vertex_table = "cleaned_ways_vertices_pgr";
	int num_levels, num_iterations, num_connections, source;
	int num_pairs, min_level, max_level, bucket_size, postgres_level, comp_id, cut_id;
	if (argc < 2) {
		std::cout << "Invalid arguments" << std::endl;
		return 0;
	}
	dbname = argv[1];
	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;
	

	// Updates the components for skeletal vertices
    skeletal_v_update_sql = "UPDATE %s \
    SET component_%s = 1 \
    FROM %s AS edges \
    WHERE (%s.id = edges.source OR %s.id = edges.target) AND edges.promoted_level <= %s;";

    // Vertices other than skeleton
    non_skeletal_v_sql = "SELECT array_agg(id) FROM %s WHERE component_%s != 1";

    // All edges except skeletal edges and edges connecting the skeleton
    residue_sql = "SELECT id, source, target, cost \
    FROM %s  \
    WHERE source != ALL(%s) AND target != ALL(%s)";
    
    // Components of the residual network
    comp_sql = "SELECT component, array_agg(node) AS vids \
    FROM pgr_connectedComponents(%s) GROUP BY component;";

	// Updates the component_ids of in component edges
	non_skeletal_e_update_sql = "UPDATE %s \
	SET component_%s = %s \
	WHERE (source = ANY(%s) \
	AND target = ANY(%s));";


	// Updates the components for non skeletal vertices
	non_skeletal_v_update_sql = "UPDATE %s \
	SET component_%s = %s \
	WHERE id = ANY(%s);";


	// Update the cut_edges 
    /*
	Cut edges: edges joining the skeletal node and a non skeletal node
    */
    cut_e_update_sql = "UPDATE %s \
	SET component_%s = %s \
	WHERE (source = ANY(%s) \
	OR target = ANY(%s)) AND component_%s != 1 AND component_%s != %s";

	

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;
	min_level = max_level = -1;
	if (num_levels > world.size())
		bucket_size = num_levels/(world.size());
	else
		bucket_size = 1;

	min_level = world.rank()*bucket_size+1;
	max_level = min_level + bucket_size;
	if (world.rank() == world.size()-1) {
		max_level += (num_levels%world.size());
	}

	
	try {
		conn_str = "dbname = %s user = postgres password = postgres \
		hostaddr = 127.0.0.1 port = 5432";
		conn_str = (boost::format(conn_str) % dbname).str().c_str();
		pqxx::connection C(conn_str.c_str());
		if (C.is_open()) {
			std::vector<long int> component_ids;
			std::vector<std::string> component_vertices_array;
			//std::cout << "min_level: " << min_level << ", max_level: " << max_level << std::endl;

			for (int i = min_level; i < max_level && i <= num_levels; ++i) {
				std::cout << "curr_level: " << i << ", rank " << world.rank() << std::endl;
				// Update the vertices of skeleton
				pqxx::work W(C);
				temp = (boost::format(skeletal_v_update_sql) %vertex_table %i
					%edge_table %vertex_table %vertex_table %i).str();
				W.exec( temp.c_str() );
				W.commit();

				//Fetching the non skeletal vertices
				pqxx::work N1(C);
				temp = (boost::format(non_skeletal_v_sql) %vertex_table
					%i).str();
				pqxx::result R1( N1.exec( temp.c_str() ));
				N1.commit();

				for (pqxx::result::const_iterator c = R1.begin(); c != R1.end(); ++c) {
					if (c[0].size() == 0) {
						non_skeletal_vertices = "";
						break;
					}
					non_skeletal_vertices = c[0].as<std::string>();
					non_skeletal_vertices.erase(0, 1);
					non_skeletal_vertices.erase(non_skeletal_vertices.size() - 1);
					non_skeletal_vertices = "ARRAY["+non_skeletal_vertices+"]";
				}

				if (non_skeletal_vertices.size() > 0) {
					//Extracting the components from residue network
					pqxx::work N2(C);
					temp = (boost::format(residue_sql) %edge_table %non_skeletal_vertices
						%non_skeletal_vertices).str();
					temp = (boost::format(comp_sql) 
						%N2.quote(temp)).str();
					pqxx::result R2( N2.exec( temp.c_str() ));
					N2.commit();

					for (pqxx::result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
						component_ids.push_back(c[0].as<long int>());
						temp = c[1].as<std::string>();
						temp.erase(0, 1);
						temp.erase(temp.size() - 1);
						temp = "ARRAY["+temp+"]";
						component_vertices_array.push_back(temp);
					}
					for (int j = 0; j < component_ids.size(); ++j) {
						// Updating the component ids of the edges
						pqxx::work W1(C);
						comp_id = component_ids[j]+1;
						cut_id = -comp_id;
						temp = (boost::format(non_skeletal_e_update_sql) %edge_table 
							%i %comp_id %component_vertices_array[j] 
							%component_vertices_array[j]).str();
						W1.exec(temp.c_str());
						W1.commit();


						// Updating the component ids of the nodes
						pqxx::work W2(C);
						temp = (boost::format(non_skeletal_v_update_sql) %vertex_table 
							%i %comp_id %component_vertices_array[j]).str();
						W2.exec(temp.c_str());
						W2.commit();


						// Updating the component ids cut edges
						pqxx::work W3(C);
		                temp = (boost::format(cut_e_update_sql) %edge_table
		                        %i %cut_id %component_vertices_array[j] %component_vertices_array[j]
		                         %i %i %comp_id).str();
		                W3.exec(temp.c_str());
		                W3.commit();

					}
				}
				


			}
					//return 1;
		} else {
			std::cout << "Can't open database" << std::endl;
					//return -1;
		}
		C.disconnect ();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
				//return -1;
	}

	

	return 0;
}
