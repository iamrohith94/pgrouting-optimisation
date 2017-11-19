psql -U postgres $1 -v file_name=$1 < sql/populate_betweenness.sql

