# Command for unlimited process
mpirun -np 50 -use-hwthread-cpus --oversubscribe hello_mpil

# Command for 1

mpicc -o 1 1.c
mpirun -np 4 ./1 K M N P

# Command for 2

mpicc -o 2 2.c
mpirun -np 4 ./2 input.txt

# Command for 3

mpicc -o 3 3.c
mpirun -np 4 ./3 input.txt
