#ifndef HELPER_H
#define HELPER_H

#include <dirent.h>
#include <string.h>
#include <omp.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include "grayscale.h"
#include "sobel.h"

// Courtesy of stack overflow: https://stackoverflow.com/a/7430262
void create_output_directory(const char *output_dir_name) {
    
    struct stat st = {0};
    if (stat(output_dir_name, &st) == -1) {
        mkdir(output_dir_name, 0700);
    }

}

void process_image_serial(const char *image_path, const char *image_processing_algorithm) {
    int width, height, channel;
    // This function reads the image and stores the widht, height, channel into the variables we defined.
    unsigned char *img, *sobel_img;
    if (!strcmp(image_processing_algorithm, "sobel")) 
    {
        printf("Sobel algorithm chosen!\n");
        sobel_img = stbi_load(image_path, &width, &height, &channel, 1);
    }
    else
    {
        img = stbi_load(image_path, &width, &height, &channel, 0);
    }

    if (img == NULL && sobel_img == NULL) {
        fprintf(stderr, "Error: Could not load image %s\n", image_path);
        return;
    }

    unsigned char *output = (unsigned char *)malloc(width * height * channel);
    const char* image_name = strrchr(image_path, '/');
    char output_dir_name[256] = "output_folder/sobel_serial";
    const char* output_dir = strcat(output_dir_name, image_name);
    printf("Loaded image: %s\n", image_path);

    // Perform some processing here
    if (!strcmp(image_processing_algorithm, "grayscale")) 
    {
        grayscale_serial(img, output, width, height, channel, "grayscale", image_name);
        stbi_image_free(img);
    }
    else if (!strcmp(image_processing_algorithm, "sobel")) 
    {
        sobel_filter(sobel_img, output, width, height);
        // Save the image
        printf("Saving image to: %s\n", output_dir);
        create_output_directory("output_folder/sobel_serial/");
        stbi_write_png(output_dir, width, height, 1, output, width);
        stbi_image_free(sobel_img);
    }

}

void read_images_from_folder_serial(const char *folder_path, const char *image_processing_algorithm) {
    struct dirent *entry;

    DIR *dp = opendir(folder_path);
    if (dp == NULL) {
        perror("Error opening the directory\n");
        return;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            const char *file_name = entry->d_name; // Get the file name
            const char *extension = strrchr(file_name, '.');

            if (extension && strcmp(extension, ".png") == 0) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, file_name);
                process_image_serial(full_path, image_processing_algorithm);
            }
        }
    }

    closedir(dp);
}

void process_image_omp(const char* image_path, const char* image_processing_algorithm) {
    int width, height, channels;
    const char* image_name = strrchr(image_path, '/');
    char output_dir_name[256] = "output_folder/sobel_omp";

    if (!strcmp(image_processing_algorithm, "grayscale"))
    {
        unsigned char *img = stbi_load(image_path, &width, &height, &channels, 0);
        if (img == NULL) {
            fprintf(stderr, "Error: Could not load image %s\n", image_path);
            return;
        }

        printf("Thread %d: Loaded image: %s (Width: %d, Height: %d, Channels: %d)\n",
           omp_get_thread_num(), image_path, width, height, channels);
        
        unsigned char *output = (unsigned char *)malloc(width * height * channels);
        grayscale_openmp(img, output, width, height, channels, "output_folder/grayscale_omp", image_name);
        stbi_image_free(img);  // Free memory when done
    }
    else if (!strcmp(image_processing_algorithm, "sobel"))
    {
        unsigned char *sobel_img = stbi_load(image_path, &width, &height, &channels, 1);

        if (sobel_img == NULL) {
            fprintf(stderr, "Error: Could not load image %s\n", image_path);
            stbi_image_free(sobel_img);
            return;
        }

        printf("Thread %d: Loaded image: %s (Width: %d, Height: %d, Channels: %d)\n",
           omp_get_thread_num(), image_path, width, height, channels);


        unsigned char *output = (unsigned char*)malloc(width * height);
        sobel_filter_omp(sobel_img, output, width, height);
        create_output_directory(output_dir_name);
        const char* output_dir = strcat(output_dir_name, image_name);
        printf("Saving to %s\n", output_dir);
        stbi_write_png(output_dir, width, height, 1, output, width);

        stbi_image_free(sobel_img);
    }

}

void read_images_from_folder_omp(const char *folder_path, const char *image_processing_algorithm) {

    struct dirent **file_list;
    int n_files = scandir(folder_path, &file_list, NULL, alphasort);

    if (n_files < 0) {
        perror("Error opening the directory");
        return;
    }

    #pragma omp parallel for schedule(dynamic) shared(file_list, folder_path)
    for (int i = 0; i < n_files; i++) {
        if (file_list[i]->d_type == DT_REG) {  // Regular file
            const char *file_name = file_list[i]->d_name;
            const char *extension = strrchr(file_name, '.');

            if (extension && strcmp(extension, ".png") == 0) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, file_name);
                process_image_omp(full_path, image_processing_algorithm);
            }
        }

        free(file_list[i]);  // Free memory for each directory entry
    }

    free(file_list);  // Free memory for the file list
}


// Helper function which checks if a file is an image by extension
int is_image_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    return strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0;
}


char **read_filenames_mpi(const char *folder, int *num_files) {
    DIR *dir;
    struct dirent *ent;
    char **filenames = NULL;
    int count = 0;

    if ((dir = opendir(folder)) != NULL) {
        // Count the number of files
        while ((ent = readdir(dir)) != NULL) {
            if (is_image_file(ent->d_name)) {
                count++;
            }
        }
        rewinddir(dir);

        filenames = (char **)malloc(count * sizeof(char *));
        int index = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (is_image_file(ent->d_name)) {
                filenames[index] = strdup(ent->d_name);
                index++;
            }
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
        exit(EXIT_FAILURE);
    }
    *num_files = count;
    printf("Files Read\n");
    return filenames;
}



void read_images_from_folders_mpi(const char *folder_path, const char *image_processing_algorithm) {
    int rank, total_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &total_processes);

    // All processes read the list of filenames
    char **filenames = NULL;
    int num_files = 0;

    if (rank == 0) {
        // Root process reads filenames
        filenames = read_filenames_mpi(folder_path, &num_files);
    }

    // Broadcast the number of files to all processes
    MPI_Bcast(&num_files, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        // Allocate memory for filenames
        filenames = (char **)malloc(num_files * sizeof(char *));
    }
    // Broadcast filenames to all processes
    for (int i = 0; i < num_files; i++) {
        int length;
        if (rank == 0) {
            length = strlen(filenames[i]) + 1;
        }
        MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank != 0) {
            filenames[i] = (char *)malloc(length * sizeof(char));
        }
        MPI_Bcast(filenames[i], length, MPI_CHAR, 0, MPI_COMM_WORLD);
    }

    // Each process processes a subset of images
    for (int i = rank; i < num_files; i += total_processes) {
        printf("Rank %d is assigned image: %s\n", rank, filenames[i]);
        fflush(stdout);
        process_image_mpi(filenames[i], folder_path);
    }

    // Clean up
    for (int i = 0; i < num_files; i++) {
        free(filenames[i]);
    }
    free(filenames);
}

char **read_filenames_sobel(const char *folder, int *num_files) {
    DIR *dir;
    struct dirent *ent;
    char **filenames = NULL;
    int count = 0;

    if ((dir = opendir(folder)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (is_image_file(ent->d_name)) {
                filenames = (char **)realloc(filenames, (count + 1) * sizeof(char *));
                filenames[count] = (char *)malloc((strlen(ent->d_name) + 1) * sizeof(char));
                strcpy(filenames[count], ent->d_name);
                count++;
            }
        }
        closedir(dir);
    } else {
        perror("Could not open directory");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    *num_files = count;
    return filenames;
}


void read_images_from_folders_mpi_sobel(const char *folder_path, const char *image_processing_algorithm) {
    int rank, size;
    char **filenames = NULL;
    int num_files = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // Root process reads the filenames
        filenames = read_filenames_sobel(folder_path, &num_files);
    }

    // Broadcast the number of files to all processes
    MPI_Bcast(&num_files, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Distribute filenames among processes
    int *sendcounts = NULL;
    int *displs = NULL;
    char *all_filenames = NULL;
    int total_length = 0;

    if (rank == 0) {
        // Calculate the total length of all filenames
        for (int i = 0; i < num_files; i++) {
            total_length += strlen(filenames[i]) + 1; // +1 for null terminator
        }

        // Concatenate all filenames into a single buffer
        all_filenames = (char *)malloc(total_length * sizeof(char));
        char *ptr = all_filenames;
        for (int i = 0; i < num_files; i++) {
            strcpy(ptr, filenames[i]);
            ptr += strlen(filenames[i]) + 1;
        }

        // Prepare sendcounts and displs for scattering
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        int filenames_per_proc = num_files / size;
        int remainder = num_files % size;
        int offset = 0;

        for (int i = 0; i < size; i++) {
            int count = filenames_per_proc + (i < remainder ? 1 : 0);
            sendcounts[i] = count;
            displs[i] = offset;
            offset += count;
        }

        // Adjust sendcounts and displs for character data
        for (int i = 0; i < size; i++) {
            int count = 0;
            int start = displs[i];
            int end = start + sendcounts[i];
            for (int j = start; j < end; j++) {
                count += strlen(filenames[j]) + 1;
            }
            sendcounts[i] = count;
        }

        // Update displs to be in bytes
        displs[0] = 0;
        for (int i = 1; i < size; i++) {
            displs[i] = displs[i - 1] + sendcounts[i - 1];
        }
    }

    // Broadcast total_length to all processes
    MPI_Bcast(&total_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process receives its portion of filenames
    int local_filenames_length = 0;
    MPI_Scatter(sendcounts, 1, MPI_INT, &local_filenames_length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    char *local_filenames = (char *)malloc(local_filenames_length * sizeof(char));
    MPI_Scatterv(all_filenames, sendcounts, displs, MPI_CHAR,
                 local_filenames, local_filenames_length, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Split local_filenames into an array
    char **local_filename_list = NULL;
    int local_file_count = 0;
    char *ptr = local_filenames;
    while (ptr < local_filenames + local_filenames_length) {
        local_filename_list = (char **)realloc(local_filename_list, (local_file_count + 1) * sizeof(char *));
        local_filename_list[local_file_count] = ptr;
        ptr += strlen(ptr) + 1;
        local_file_count++;
    }

    // Each process processes its assigned images
    for (int i = 0; i < local_file_count; i++) {
        printf("Rank %d is processing image: %s\n", rank, local_filename_list[i]);
        fflush(stdout);
        sobel_filter_hybrid(local_filename_list[i], folder_path ,"output_folder");
    }

    // Clean up
    if (rank == 0) {
        for (int i = 0; i < num_files; i++) {
            free(filenames[i]);
        }
        free(filenames);
        free(all_filenames);
        free(sendcounts);
        free(displs);
    }
    free(local_filenames);
    free(local_filename_list);
}

#endif