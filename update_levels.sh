for i in `seq $3`;
do
	echo $i
	mpirun -n $2 ./p_l $1 $i
done



