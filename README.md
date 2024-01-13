# Command for more than 4 process
mpirun -np 50 -use-hwthread-cpus --oversubscribe filename

# Command for 1

<p>mpicc -o 1 1.c</p>
mpirun -np 4 ./1 K M N P

# Command for 2

<p>mpicc -o 2 2.c</p>
mpirun -np 4 ./2 input.txt

# Command for 3

<p>mpicc -o 3 3.c</p>
mpirun -np 4 ./3 input.txt
