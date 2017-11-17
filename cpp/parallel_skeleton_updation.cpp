#include <boost/mpi.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, comp_sql, residue_sql,
	v_update_sql_2, edges_sql;
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
			//std::cout << "min_level: " << min_level << ", max_level: " << max_level << std::endl;

			for (int i = min_level; i < max_level && i <= num_levels; ++i) {
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
