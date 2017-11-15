#include <boost/mpi.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <pqxx/pqxx> 
#include <boost/format.hpp>
int main(int argc, char *argv[])
{
	std::string conn_str, temp, dbname, edge_table, vertex_table, skeleton_sql, connections_sql, drop_sql,
	update_sql, select_sql, targets, edges_sql;
	edge_table = "cleaned_ways";
	vertex_table = "cleaned_ways_vertices_pgr";
	int num_levels = 10, num_iterations = 3, num_connections, source;
	int num_pairs, curr_level;
	if (argc < 3) {
		std::cout << "Invalid arguments" << std::endl;
		return 0;
	}
	dbname = argv[1];
	curr_level = std::atoi(argv[2]);
	edges_sql = "SELECT id, source, target, cost FROM %s";
	edges_sql = (boost::format(edges_sql) % edge_table).str();
	skeleton_sql = "SELECT id, source, target, cost FROM %s WHERE promoted_level <= %s";
	connections_sql = "SELECT source, array_agg(target)::bigint[] as targets INTO connections \
			FROM pgr_makeConnected(%s) GROUP BY source;";
	drop_sql = "DROP TABLE IF EXISTS connections";


	update_sql = "UPDATE %s SET promoted_level = %s WHERE id IN \
	(SELECT DISTINCT(edge) FROM pgr_dijkstra(%s, %s, %s) WHERE edge != -1)  \
	AND promoted_level > %s;";


	select_sql = "SELECT source, targets FROM connections LIMIT %s OFFSET %s";

	boost::mpi::environment env(argc, argv);
	boost::mpi::communicator world;
	if (world.rank() == 0) {

		std::cout << "dbname: " << dbname << ", level: " << curr_level << std::endl;
		try {
			conn_str = "dbname = %s user = postgres password = postgres \
			hostaddr = 127.0.0.1 port = 5432";
			conn_str = (boost::format(conn_str) % dbname).str().c_str();
			pqxx::connection C(conn_str.c_str());
			if (C.is_open()) {
				pqxx::work W1(C);
				W1.exec(drop_sql.c_str());
				W1.commit();
				
				pqxx::work W2(C);
				skeleton_sql = (boost::format(skeleton_sql) 
				% edge_table % curr_level).str();
				connections_sql = (boost::format(connections_sql) 
				% W2.quote(skeleton_sql)).str();
				//std::cout << "connections_query: " << connections_sql  << std::endl;
				pqxx::result cnt = W2.exec(connections_sql.c_str());
				num_connections = cnt.affected_rows();
				std::cout << "affected_rows: " <<  num_connections << std::endl;
				W2.commit();
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
		boost::mpi::broadcast(world, num_connections, 0);
	}
	else {
		std::vector<long int> sources;
		std::vector<std::string> targets_array;
		// After insertion run a parallel query to update the promoted levels
		boost::mpi::broadcast(world, num_connections, 0);
		int bucket_size, offset;

		if (num_connections > world.size()-1)
			bucket_size = num_connections/(world.size()-1);
		else
			bucket_size = 1;
		offset = (world.rank()-1)*bucket_size;
		try {
			conn_str = "dbname = %s user = postgres password = postgres \
			hostaddr = 127.0.0.1 port = 5432";
			conn_str = (boost::format(conn_str) % dbname).str().c_str();
			pqxx::connection C(conn_str.c_str());
			if (C.is_open()) {
				pqxx::work N(C);
				select_sql = (boost::format(select_sql) 
				% bucket_size % offset).str();
				pqxx::result R2( N.exec( select_sql.c_str() ));
				//N.commit();
				for (pqxx::result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
					sources.push_back(c[0].as<long int>());
					targets = c[1].as<std::string>();
					targets.erase(0, 1);
					targets.erase(targets.size() - 1);
					targets = "ARRAY["+targets+"]";
					targets_array.push_back(targets);
				}
				N.commit();
				std::cout << "num connections: " << sources.size() 
				<< ", rank: " << world.rank() << std::endl;
				for (int i = 0; i < sources.size(); ++i) {
					
					//std::cout << "source: " << source << ", targets: "
					//<< targets << ", rank: " << world.rank() << std::endl;
					//Updating promoted level of edges	
					pqxx::work W(C);
					update_sql = (boost::format(update_sql) 
					%edge_table %curr_level %W.quote(edges_sql) 
					%sources[i] %targets_array[i] %curr_level).str();
					//std::cout << "update_query: " << update_sql  << std::endl;
					W.exec(update_sql.c_str());
					W.commit();
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
	}
	return 0;
}