#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/helper.h"
#include <time.h>

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("No image folder provided: ./main <image folder path> serial | omp | mpi <algorithm>\n");
        printf("Possible image processing algorithms are:\n1. sobel\n2. grayscale\n3. negative\n4. blur\n5. sepia\n");
        return 1;
    }

    clock_t start, finish;
    double serial_processing_time;
    double omp_start, omp_finish, omp_processing_time;
    const char *folder_path = argv[1];
    const char *execution_type = argv[2];
    const char *image_processing_algorithm = argv[3];
    printf("The image path provided is: %s\n", folder_path);
    printf("Running algorithm %s on images\n", image_processing_algorithm);
    if (strcmp(execution_type, "serial") == 0) 
    {
        start = clock();
        read_images_from_folder_serial(folder_path, image_processing_algorithm);
        finish = clock();

        serial_processing_time = (double)(finish - start) / CLOCKS_PER_SEC;
        printf("Total time taken to apply %s filter on 100 images: %lf\n", image_processing_algorithm, serial_processing_time);
    } 
    else if (strcmp(execution_type, "omp") == 0) 
    {
        omp_start = omp_get_wtime();

        read_images_from_folder_omp(folder_path, image_processing_algorithm);

        omp_finish = omp_get_wtime();

        omp_processing_time = (omp_finish - omp_start);
        printf("Total time taken to apply %s on 100 images: %lf\n", image_processing_algorithm, omp_processing_time);
    } 
    else if (strcmp(execution_type, "mpi") == 0) 
    {
        // Initialize MPI
        if (!strcmp(image_processing_algorithm, "grayscale"))
        {
            MPI_Init(&argc, &argv);

            read_images_from_folders_mpi(folder_path, image_processing_algorithm);

            MPI_Finalize();
        }
        else if (!strcmp(image_processing_algorithm, "sobel"))
        {
            MPI_Init(&argc, &argv);
            
            read_images_from_folders_mpi_sobel(folder_path, image_processing_algorithm);

            MPI_Finalize();
        }
    }
    return 0;
}