#! /bin/bash

echo "Choose method of program execution";

if [[ -z $1 || -z $2 ]]; then
    echo "Provide image folder: ./run.sh <image_path> serial | omp | mpi <procs>";
    exit 1;
fi

if [[ $2 == 'serial' ]]; then
    mpicc main.c libs/grayscale.c -o build/main_serial -lm -fopenmp
    ./build/main_serial $1 serial
    exit 0;
elif [[ $2 == 'omp' ]]; then
    mpicc main.c libs/grayscale.c -o build/main_omp -lm -fopenmp
    ./build/main_omp $1 omp
    exit 0;
elif [[ $2 == 'mpi' ]]; then
    mpicc main.c libs/grayscale.c -o build/main_mpi -lm -fopenmp
    mpirun -np $3 ./build/main_mpi $1 mpi
    exit 0;
else
    echo "Incorrect last argument: serial | omp | mpi";
    exit 1;
fi
