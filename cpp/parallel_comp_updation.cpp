#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, comp_sql, residue_sql,
		skeletal_v_update_sql, non_skeletal_v_update_sql, cut_e_update_sql,  skeletal_e_update_sql, 
		non_skeletal_e_update_sql, edges_sql, non_skeletal_v_sql, non_skeletal_vertices, initialise_sql,
		unassigned_vertices, unassigned_vertex_update, unassigned_edge_update, skeletal_e_id_update,
		skeletal_v_sql;
	edge_table = "cleaned_ways";
	vertex_table = "cleaned_ways_vertices_pgr";
	int num_levels, num_iterations, num_connections, source;
	int num_pairs, min_level, max_level, bucket_size, postgres_level, comp_id, cut_id, temp_id;
	if (argc < 2) {
		std::cout << "Invalid arguments" << std::endl;
		return 0;
	}
	dbname = argv[1];
	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;
	
	//Initialise the component ids to 0
	initialise_sql = "UPDATE %s \
			    SET component_%s = 0";


	// Updates the component id for skeletal edges
	skeletal_e_update_sql = "UPDATE %s \
				 SET component_%s = 1 \
				 WHERE promoted_level <= %s;";

	skeletal_v_sql = "SELECT id FROM %s WHERE component_%s = 1";


	// Updates the component id for skeletal vertices
        skeletal_v_update_sql = "UPDATE %s \
                                 SET component_%s = 1 \
                                 FROM %s AS edges \
                                 WHERE %s.id = %s.parent AND (%s.id = edges.source OR %s.id = edges.target) AND edges.component_%s = 1;";

	//Update skeletal edges with same id
        skeletal_e_id_update = "UPDATE %s \
                                SET component_%s = 1 \
				WHERE source = ANY(ARRAY(%s)) AND target = ANY(ARRAY(%s)) AND component_%s != 1";


	// Vertices other than skeleton
	non_skeletal_v_sql = "SELECT id FROM %s WHERE component_%s != 1 AND id = parent";

	// All edges except skeletal edges and edges connecting the skeleton
	residue_sql = "SELECT id, source, target, cost \
		       FROM %s  \
		       WHERE source = ANY(ARRAY(%s)) AND target = ANY(ARRAY(%s))";

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


	unassigned_vertices = "SELECT id FROM %s WHERE component_%s = 0";

	unassigned_vertex_update = "UPDATE %s SET component_%s = %s WHERE id = %s";

	unassigned_edge_update = "UPDATE %s SET component_%s = %s WHERE source = %s OR target = %s";



	try {
		conn_str = "dbname = %s user = postgres password = postgres \
			    hostaddr = 127.0.0.1 port = 5432";
		conn_str = (boost::format(conn_str) % dbname).str().c_str();
		pqxx::connection C(conn_str.c_str());
		if (C.is_open()) {
			std::vector<long int> component_ids;
			std::vector<std::string> component_vertices_array;
			//std::cout << "min_level: " << min_level << ", max_level: " << max_level << std::endl;

			for (int i = 1; i <= num_levels; ++i) {
				std::cout << "curr_level: " << i  << std::endl;
				component_ids.clear();
				component_vertices_array.clear();				
				//Initialisation
				pqxx::work I0(C);
                                temp = (boost::format(initialise_sql) %edge_table %i).str();
                                I0.exec( temp.c_str() );
                                I0.commit();				

				pqxx::work I1(C);
                                temp = (boost::format(initialise_sql) %vertex_table %i).str();
                                I1.exec( temp.c_str() );
                                I1.commit();


				// Update the edges of skeleton
                                pqxx::work N0(C);
                                temp = (boost::format(skeletal_e_update_sql) %edge_table %i %i).str();
                                N0.exec( temp.c_str() );
                                N0.commit();	


				// Update the vertices of skeleton
				pqxx::work W(C);
				temp = (boost::format(skeletal_v_update_sql) %vertex_table %i
						%edge_table %vertex_table %vertex_table  %vertex_table %vertex_table %i).str();
				W.exec( temp.c_str() );
				W.commit();
				#if 1
				//Update edges with both source and target being skeletal nodes
				pqxx::work W0(C);
				temp = (boost::format(skeletal_v_sql) %vertex_table %i).str();
                                temp = (boost::format(skeletal_e_id_update) %edge_table %i
                                                 %temp %temp %i).str();
                                W0.exec( temp.c_str() );
                                W0.commit();
				#endif

					
				//Extracting the components from residual network
				pqxx::work N2(C);
				temp = (boost::format(non_skeletal_v_sql) %vertex_table %i).str();
				temp = (boost::format(residue_sql) %edge_table %temp
						%temp).str();
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
					#if 1
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
					#endif

				}
				// Updating the remaining components with one vertex
				pqxx::work U0(C);
                                temp = (boost::format(unassigned_vertices) %vertex_table %i).str();
				pqxx::result UR( U0.exec( temp.c_str() ));
                                U0.commit();

                                for (pqxx::result::const_iterator c = UR.begin(); c != UR.end(); ++c) {
					temp_id = c[0].as<long int>();
					++comp_id;
					cut_id = -comp_id;
					pqxx::work U1(C);
                                	temp = (boost::format(unassigned_vertex_update) %vertex_table %i %comp_id %temp_id).str();
                                	U1.exec( temp.c_str() );
                                	U1.commit();
					pqxx::work U2(C);
                                        temp = (boost::format(unassigned_edge_update) %edge_table %i %cut_id %temp_id %temp_id).str();
                                        U2.exec( temp.c_str() );
                                        U2.commit();
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
