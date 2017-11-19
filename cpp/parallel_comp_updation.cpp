#include <boost/mpi.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, comp_sql, residue_sql,
	e_update_sql, v_update_sql_2, v_update_sql_1, edges_sql;
	edge_table = "cleaned_ways";
	vertex_table = "cleaned_ways_vertices_pgr";
	int num_levels, num_iterations, num_connections, source;
	int num_pairs, min_level, max_level, bucket_size, postgres_level, comp_id;
	if (argc < 2) {
		std::cout << "Invalid arguments" << std::endl;
		return 0;
	}
	dbname = argv[1];
	if (argc == 3)
		num_levels = std::atoi(argv[2]);
	else
		num_levels = 10;
	residue_sql = "SELECT id, source, target, cost FROM %s WHERE promoted_level > %s";
	comp_sql = "SELECT component, array_agg(node) AS vids \
	FROM pgr_connectedComponents(%s) GROUP BY component;";
	// Updates the components in edge table
	e_update_sql = "UPDATE %s \
	SET component_%s = %s \
	WHERE (source = ANY(%s) \
	OR target = ANY(%s)) \
	AND promoted_level > %s;";

	// Updates the components for non skeletal vertices
	v_update_sql_1 = "UPDATE %s \
	SET component_%s = %s \
	WHERE id = ANY(%s);";

	// Updates the components for skeletal vertices
        v_update_sql_2 = "UPDATE %s \
        SET component_%s = 1 \
        FROM %s AS edges \
        WHERE (%s.id = edges.source OR %s.id = edges.target) AND edges.promoted_level <= %s;";

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
				pqxx::work N(C);
				temp = (boost::format(residue_sql) %edge_table 
					%i).str();
				temp = (boost::format(comp_sql) 
					%N.quote(temp)).str();
				pqxx::result R2( N.exec( temp.c_str() ));
					//N.commit();
				for (pqxx::result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
					component_ids.push_back(c[0].as<long int>());
					temp = c[1].as<std::string>();
					temp.erase(0, 1);
					temp.erase(temp.size() - 1);
					temp = "ARRAY["+temp+"]";
					component_vertices_array.push_back(temp);
				}
				N.commit();
				for (int j = 0; j < component_ids.size(); ++j) {
					
					pqxx::work W1(C);
					comp_id = component_ids[j]+1;
					temp = (boost::format(e_update_sql) %edge_table 
						%i %comp_id %component_vertices_array[j] 
						%component_vertices_array[j] %i).str();
					W1.exec(temp.c_str());
					W1.commit();

					
					pqxx::work W2(C);
					temp = (boost::format(v_update_sql_1) %vertex_table 
						%i %comp_id %component_vertices_array[j]).str();
					W2.exec(temp.c_str());
					W2.commit();


				}
				pqxx::work W3(C);
                                        temp = (boost::format(v_update_sql_2) %vertex_table
                                                %i %edge_table %vertex_table %vertex_table %i).str();
                                        W3.exec(temp.c_str());
                                        W3.commit();


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
