#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION // Need this for stb_image.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/helper.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        printf("No image folder provided: ./main <image folder path> serial | omp | mpi <algorithm>\n");
        printf("Possible image processing algorithms are:\n1. sobel\n2. grayscale\n3. negative\n4. blur\n5. sepia\n");
        return 1;
    }
    const char *folder_path = argv[1];
    const char *execution_type = argv[2];
    const char *image_processing_algorithm = argv[3];
    printf("The image path provided is: %s\n", folder_path);
    printf("Running algorithm %s on images\n", image_processing_algorithm);
    if (strcmp(execution_type, "serial") == 0) 
    {
        read_images_from_folder_serial(folder_path, image_processing_algorithm);
    } 
    else if (strcmp(execution_type, "omp") == 0) 
    {
        read_images_from_folder_omp(folder_path, image_processing_algorithm);
    } 
    else if (strcmp(execution_type, "mpi") == 0) 
    {
        // Initialize MPI
        MPI_Init(&argc, &argv);

        read_images_from_folders_mpi(folder_path, image_processing_algorithm);

        MPI_Finalize();
    }
    return 0;
}