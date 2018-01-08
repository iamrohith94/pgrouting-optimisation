#include <boost/mpi.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, 
	insert_sql, performance_table, scc_edge_table;
	edge_table = "cleaned_ways";
	vertex_table = "cleaned_ways_vertices_pgr";
	scc_edge_table = "scc_ways";
	performance_table = "level_wise_performance";
	int num_levels, num_sets, pairs_per_set;
	if (argc < 5) {
		std::cout << "Invalid arguments" << std::endl;
		return 0;
	}
	dbname = argv[1];
	num_levels = std::atoi(argv[2]);
	pairs_per_set = std::atoi(argv[3]);
	num_sets = std::atoi(argv[4]);

	// Insert query to insert pairs for performance analysis
	// Carried out by every process
	insert_sql = "INSERT INTO %s \
	SELECT * FROM getlevelwiseperformanceanalysis(%s, %s, %s, %s, %s, %s)";

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;

	try {
		conn_str = "dbname = %s user = postgres password = postgres \
		hostaddr = 127.0.0.1 port = 5432";
		conn_str = (boost::format(conn_str) % dbname).str().c_str();
		pqxx::connection C(conn_str.c_str());
		if (C.is_open()) {
			pqxx::work W(C);
			temp = (boost::format(insert_sql) %performance_table %W.quote(edge_table) %W.quote(scc_edge_table)
			%W.quote(vertex_table) %num_levels %pairs_per_set %num_sets).str();
			W.exec(temp.c_str());
			W.commit();
		} else {
			std::cout << "Can't open database" << std::endl;
		}
		C.disconnect ();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
