#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/helper.h"
#include <time.h>

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("No image folder provided: ./main <image folder path> serial | omp | mpi <algorithm>\n");
        printf("Possible image processing algorithms are:\n1. sobel\n2. grayscale\n3. negative\n4. blur\n5. otsu\n");
        return 1;
    }

    clock_t start, finish;
    double serial_processing_time;
    double omp_start, omp_finish, omp_processing_time;
    double mpi_start, mpi_finish, mpi_processing_time;
    const char *folder_path = argv[1];
    const char *execution_type = argv[2];
    const char *image_processing_algorithm = argv[3];

    int otsu_threshold = 0;
    if (!strcmp(image_processing_algorithm, "otsu"))
    {
        printf("Please provide a threshold for otsu binarization (0 - 255): ");
        scanf("%d", &otsu_threshold);
        if (otsu_threshold < 0 || otsu_threshold > 255)
        {
            printf("Invalid value range. Image should be between 0 - 255. Current value provided: %d\n", otsu_threshold);
            return 1;
        }
    }

    printf("The image path provided is: %s\n", folder_path);
    printf("Running algorithm %s on images\n", image_processing_algorithm);
    if (strcmp(execution_type, "serial") == 0) 
    {
        start = clock();
        read_images_from_folder_serial(folder_path, image_processing_algorithm, otsu_threshold);
        finish = clock();

        serial_processing_time = (double)(finish - start) / CLOCKS_PER_SEC;
        printf("Total time taken to apply %s filter on 100 images: %lf\n", image_processing_algorithm, serial_processing_time);
    } 
    else if (strcmp(execution_type, "omp") == 0) 
    {
        omp_start = omp_get_wtime();

        read_images_from_folder_omp(folder_path, image_processing_algorithm, otsu_threshold);

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
            
            mpi_start = MPI_Wtime();

            read_images_from_folders_mpi(folder_path, image_processing_algorithm);

            mpi_finish = MPI_Wtime();

            MPI_Finalize();

            mpi_processing_time = mpi_finish - mpi_start;
            printf("Total time taken to apply %s on 100 images using %s method: %lf\n", image_processing_algorithm, execution_type, mpi_processing_time);
        }
        else if (!strcmp(image_processing_algorithm, "sobel"))
        {
            MPI_Init(&argc, &argv);
            
            mpi_start = MPI_Wtime();
            read_images_from_folders_mpi_sobel(folder_path, image_processing_algorithm);

            mpi_finish = MPI_Wtime();
            MPI_Finalize();

            mpi_processing_time = mpi_finish - mpi_start;
            printf("Total time taken to apply %s on 100 images using %s method: %lf\n", image_processing_algorithm, execution_type, mpi_processing_time);
        }
        else if (!strcmp(image_processing_algorithm, "negative"))
        {
            MPI_Init(&argc, &argv);
            
            MPI_Barrier(MPI_COMM_WORLD);
            mpi_start = MPI_Wtime();
            int rank = read_images_from_folders_mpi_negative(folder_path);
            MPI_Barrier(MPI_COMM_WORLD);
            mpi_finish = MPI_Wtime();
            MPI_Finalize();

            mpi_processing_time = mpi_finish - mpi_start;
            if (rank == 0) {
                printf("Total time taken to apply %s on 100 images using %s method: %lf\n", image_processing_algorithm, execution_type, mpi_processing_time);
            }
        }
    }
    return 0;
}