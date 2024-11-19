#include "grayscale.h"

// Courtesy of stack overflow: https://stackoverflow.com/a/7430262
// Create a directory if not already created
void create_output_directory(const char *output_dir_name) {
    
    struct stat st = {0};
    if (stat(output_dir_name, &st) == -1) {
        mkdir(output_dir_name, 0700);
    }

}

void grayscale_serial(unsigned char *buffer, unsigned char *output, int width, int height, int channels, const char *output_folder, const char *original_file) {
    for (int i = 0; i < width * height; i++) {
        int idx = i * channels;
        unsigned char gray = (unsigned char)(0.299 * buffer[idx] + 0.587 * buffer[idx + 1] + 0.114 * buffer[idx + 2]);
        output[idx] = output[idx + 1] = output[idx + 2] = gray; // Set RGB to grayscale
        if (channels == 4) output[idx + 3] = buffer[idx + 3];  // Preserve alpha channel if present
    }
     // Save the grayscaled image
    save_image(output_folder, original_file, output, width, height, channels);
}

void grayscale_openmp(unsigned char *buffer, unsigned char *output, int width, int height, int channels, const char* output_folder, const char *original_file) {
    #pragma omp parallel for
    for (int i = 0; i < width * height; i++) {
        int idx = i * channels;
        unsigned char gray = (unsigned char)(0.299 * buffer[idx] + 0.587 * buffer[idx + 1] + 0.114 * buffer[idx + 2]);
        output[idx] = output[idx + 1] = output[idx + 2] = gray; // Set RGB to grayscale
        if (channels == 4) output[idx + 3] = buffer[idx + 3];  // Preserve alpha channel if present
    }

    save_image(output_folder, original_file, output, width, height, channels);
}

void process_image_mpi(const char *filename, const char *input_folder) {
     int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Print which rank is processing which image
    printf("Rank %d is processing image: %s\n", rank, filename);
    fflush(stdout);

    char input_path[256];
    sprintf(input_path, "%s/%s", input_folder, filename);

    int width, height, channels;
    unsigned char *img = stbi_load(input_path, &width, &height, &channels, 0);
    if (img == NULL) {
        fprintf(stderr, "Error loading image %s\n", input_path);
        return;
    }

    // Convert to grayscale
    size_t img_size = width * height * channels;
    unsigned char *gray_img = (unsigned char *)malloc(width * height);

    for (size_t i = 0; i < width * height; i++) {
        int idx = i * channels;
        int r = img[idx];
        int g = img[idx + 1];
        int b = img[idx + 2];
        gray_img[i] = (r + g + b) / 3;
    }

    // Save the grayscale image
    char output_path[256];
    create_output_directory("output_folder/grayscale_mpi/");
    sprintf(output_path, "output_folder/grayscale_mpi/%s", filename);
    stbi_write_png(output_path, width, height, 1, gray_img, width);

    // Clean up
    stbi_image_free(img);
    free(gray_img);
}


void save_image(const char *output_folder, const char *original_file, unsigned char *output, int width, int height, int channels) {
    // Create the output folder if it doesn't exist
    struct stat st = {0};
    if (stat(output_folder, &st) == -1) {
        mkdir(output_folder, 0700);  // Create directory with permissions
    }

    // Generate the output file path
    char output_file[1024];
    snprintf(output_file, sizeof(output_file), "%s/%s", output_folder, original_file);

    // Save the image as PNG
    if (!stbi_write_png(output_file, width, height, channels, output, width * channels)) {
        fprintf(stderr, "Error: Failed to save image %s\n", output_file);
    } else {
        printf("Image saved: %s\n", output_file);
    }
}
