#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION // Need this for stb_image.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/helper.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("No image folder provided: ./main <image folder path> serial | omp | mpi\n");
        return 1;
    }
    const char *folder_path = argv[1];
    printf("The image path provided is: %s\n", folder_path);
    if (strcmp(argv[2], "serial") == 0) {
        read_images_from_folder_serial(folder_path);
    } else if (strcmp(argv[2], "omp") == 0) {
        read_images_from_folder_omp(folder_path);
    } else if (strcmp(argv[2], "mpi") == 0) {
        // Initialize MPI
        MPI_Init(&argc, &argv);

        read_images_from_folders_mpi(folder_path);

        MPI_Finalize();
    }
    return 0;
}