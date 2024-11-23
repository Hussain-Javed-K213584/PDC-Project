#include "sobel.h"
#include <stdlib.h>
#include <math.h>
#include <mpi/mpi.h>
#include <stdio.h>
#include <string.h>

// Sobel operator kernels for horizontal and vertical edge detection
const int Gx[3][3] = {
    {-1,  0,  1},
    {-2,  0,  2},
    {-1,  0,  1}
};

const int Gy[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

void sobel_filter(const unsigned char *input_image, unsigned char *output_image,
                  int width, int height) {
    // Iterate over the image excluding the border pixels
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0;
            int sumY = 0;

            // Apply the Sobel kernels to the neighborhood
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = input_image[(y + ky) * width + (x + kx)];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            // Calculate the gradient magnitude
            int magnitude = (int)sqrt((double)(sumX * sumX + sumY * sumY));

            // Normalize and clamp the result to [0, 255]
            if (magnitude > 255) {
                magnitude = 255;
            }

            output_image[y * width + x] = (unsigned char)magnitude;
        }
    }

    // Handle the border pixels by setting them to zero
    for (int x = 0; x < width; x++) {
        output_image[0 * width + x] = 0;
        output_image[(height - 1) * width + x] = 0;
    }

    for (int y = 0; y < height; y++) {
        output_image[y * width + 0] = 0;
        output_image[y * width + (width - 1)] = 0;
    }
}

// OpenMP implmentation of sobel
void sobel_filter_omp(const unsigned char *input_image, unsigned char *output_image,
                  int width, int height) {
    // Handle the border pixels by setting them to zero
    // We can parallelize these loops as well
    #pragma omp parallel for
    for (int x = 0; x < width; x++) {
        output_image[0 * width + x] = 0;
        output_image[(height - 1) * width + x] = 0;
    }

    #pragma omp parallel for
    for (int y = 0; y < height; y++) {
        output_image[y * width + 0] = 0;
        output_image[y * width + (width - 1)] = 0;
    }

    // Parallelize the main loop using OpenMP
    #pragma omp parallel for collapse(2)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0;
            int sumY = 0;

            // Apply the Sobel kernels to the neighborhood
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = input_image[(y + ky) * width + (x + kx)];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            // Calculate the gradient magnitude
            int magnitude = (int)sqrt((double)(sumX * sumX + sumY * sumY));

            // Normalize and clamp the result to [0, 255]
            if (magnitude > 255) {
                magnitude = 255;
            }

            output_image[y * width + x] = (unsigned char)magnitude;
        }
    }
}


void sobel_filter_hybrid(const char *filename, const char *input_folder, unsigned char *output_folder) {
    char input_path[256];
    sprintf(input_path, "%s/%s", input_folder, filename);

    int width, height, channels;
    unsigned char *img = stbi_load(input_path, &width, &height, &channels, 1); // Load as grayscale
    if (img == NULL) {
        fprintf(stderr, "Error loading image %s\n", input_path);
        return;
    }

    // Allocate memory for the edge-detected image
    unsigned char *edge_img = (unsigned char *)malloc(width * height);
    if (edge_img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        stbi_image_free(img);
        return;
    }

    // Apply the Sobel filter (parallelized with OpenMP)
    sobel_filter_omp(img, edge_img, width, height);

    // Save the edge-detected image
    char output_path[256];
    sprintf(output_path, "%s/edge_mpi", output_folder);
    create_output_directory(output_path);
    sprintf(output_path, "%s/edge_mpi/%s", output_folder, filename);
    if (!stbi_write_png(output_path, width, height, 1, edge_img, width)) {
        fprintf(stderr, "Error saving image %s\n", output_path);
    }

    // Clean up
    stbi_image_free(img);
    free(edge_img);
}