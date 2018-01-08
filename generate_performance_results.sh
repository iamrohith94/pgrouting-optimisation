echo 'Working on hyderabad'

time mpirun -n 8 time ./p_p_a hyd $1 $2 $3
echo 'Working on NYC'
time mpirun -n 8 time ./p_p_a nyc $1 $2 $3
echo 'Working on belgium'
time mpirun -n 8 time ./p_p_a belgium $1 $2 $3