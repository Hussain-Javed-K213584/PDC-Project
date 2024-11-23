#include "otsu.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <omp.h>

#define GRAY_LEVELS 256

int compute_otsu_threshold(const unsigned char *gray_image, int width, int height) {
    int histogram[GRAY_LEVELS] = {0};
    int total_pixels = width * height;

    // Compute histogram
    for (int i = 0; i < total_pixels; i++) {
        histogram[gray_image[i]]++;
    }

    // Total mean level
    double sum = 0;
    for (int i = 0; i < GRAY_LEVELS; i++) {
        sum += i * histogram[i];
    }

    double sumB = 0;
    int wB = 0;
    int wF = 0;

    double varMax = 0;
    int threshold = 0;

    for (int t = 0; t < GRAY_LEVELS; t++) {
        wB += histogram[t];               // Weight Background
        if (wB == 0) continue;

        wF = total_pixels - wB;           // Weight Foreground
        if (wF == 0) break;

        sumB += (double)(t * histogram[t]);

        double mB = sumB / wB;            // Mean Background
        double mF = (sum - sumB) / wF;    // Mean Foreground

        // Between Class Variance
        double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        // Check if new maximum found
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }

    return threshold;
}

void apply_threshold(const unsigned char *gray_image, unsigned char *binary_image,
                     int width, int height, int threshold) {
    int total_pixels = width * height;

    for (int i = 0; i < total_pixels; i++) {
        binary_image[i] = (gray_image[i] > threshold) ? 255 : 0;
    }
}

void otsu_serial(unsigned char *img, const char *filename, int user_threshold, int width, int height, int channels)
{
    // Convert to grayscale if necessary
    unsigned char *gray_img = NULL;
    if (channels == 1) {
        gray_img = img;
    } else {
        gray_img = (unsigned char *)malloc(width * height);
        if (gray_img == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            stbi_image_free(img);
        }
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = y * width + x;
                int idx_img = idx * channels;
                int r = img[idx_img];
                int g = img[idx_img + 1];
                int b = img[idx_img + 2];
                gray_img[idx] = (r + g + b) / 3;
            }
        }
        stbi_image_free(img);
    }

    // Compute Otsu's threshold if user_threshold is 0
    int threshold;
    if (user_threshold == 0) {
        threshold = compute_otsu_threshold(gray_img, width, height);
        printf("Computed Otsu's threshold: %d\n", threshold);
    } else {
        threshold = user_threshold;
        printf("Using user-provided threshold: %d\n", threshold);
    }

    // Apply threshold
    unsigned char *binary_img = (unsigned char *)malloc(width * height);
    if (binary_img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        if (channels != 1) free(gray_img);
    }
    apply_threshold(gray_img, binary_img, width, height, threshold);
    // Save the binary image
    char output_path[1024];
    sprintf(output_path, "output_folder/serial_otsu%s", filename);
    printf("Saving image to path: %s\n", output_path);
    if (!stbi_write_png(output_path, width, height, 1, binary_img, width)) {
        fprintf(stderr, "Error writing image %s\n", output_path);
    }

    // Clean up
    if (channels != 1) free(gray_img);
    free(binary_img);
}

int compute_otsu_threshold_omp(const unsigned char *gray_image, int width, int height) {
    int total_pixels = width * height;
    int histogram[GRAY_LEVELS] = {0};

    // Compute histogram with OpenMP
    int num_threads = omp_get_max_threads();
    int **local_histograms = (int **)malloc(num_threads * sizeof(int *));
    for (int i = 0; i < num_threads; i++) {
        local_histograms[i] = (int *)calloc(GRAY_LEVELS, sizeof(int));
    }

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int *local_hist = local_histograms[thread_id];

        #pragma omp for nowait
        for (int i = 0; i < total_pixels; i++) {
            local_hist[gray_image[i]]++;
        }
    }

    // Combine local histograms
    for (int i = 0; i < GRAY_LEVELS; i++) {
        for (int j = 0; j < num_threads; j++) {
            histogram[i] += local_histograms[j][i];
        }
    }

    // Free local histograms
    for (int i = 0; i < num_threads; i++) {
        free(local_histograms[i]);
    }
    free(local_histograms);

    // Total mean level
    double sum = 0.0;
    for (int i = 0; i < GRAY_LEVELS; i++) {
        sum += i * histogram[i];
    }

    double sumB = 0.0;
    int wB = 0;
    int threshold = 0;
    double varMax = 0.0;

    // Threshold computation remains serial due to data dependencies
    for (int t = 0; t < GRAY_LEVELS; t++) {
        wB += histogram[t];               // Weight Background
        if (wB == 0) continue;

        int wF = total_pixels - wB;       // Weight Foreground
        if (wF == 0) break;

        sumB += (double)(t * histogram[t]);

        double mB = sumB / wB;            // Mean Background
        double mF = (sum - sumB) / wF;    // Mean Foreground

        // Between Class Variance
        double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

        // Check if new maximum found
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }

    return threshold;
}

void apply_threshold_omp(const unsigned char *gray_image, unsigned char *binary_image,
                     int width, int height, int threshold) {
    int total_pixels = width * height;

    // Apply threshold with OpenMP
    #pragma omp parallel for
    for (int i = 0; i < total_pixels; i++) {
        binary_image[i] = (gray_image[i] > threshold) ? 255 : 0;
    }
}

void otsu_omp(unsigned char *img, const char *filename, int user_threshold, int width, int height, int channels)
{
    // Convert to grayscale if necessary
    unsigned char *gray_img = NULL;
    if (channels == 1) {
        gray_img = img;
    } else {
        gray_img = (unsigned char *)malloc(width * height);
        if (gray_img == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            stbi_image_free(img);
        }
        // Parallelize the grayscale conversion
        #pragma omp parallel for
        for (int idx = 0; idx < width * height; idx++) {
            int idx_img = idx * channels;
            int r = img[idx_img];
            int g = img[idx_img + 1];
            int b = img[idx_img + 2];
            gray_img[idx] = (r + g + b) / 3;
        }
        stbi_image_free(img);
    }

    // Compute Otsu's threshold if user_threshold is 0
    int threshold;
    if (user_threshold == 0) {
        threshold = compute_otsu_threshold_omp(gray_img, width, height);
        printf("Computed Otsu's threshold: %d\n", threshold);
    } else {
        threshold = user_threshold;
        printf("Using user-provided threshold: %d\n", threshold);
    }

    // Apply threshold
    unsigned char *binary_img = (unsigned char *)malloc(width * height);
    if (binary_img == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        if (channels != 1) free(gray_img);
    }
    apply_threshold_omp(gray_img, binary_img, width, height, threshold);
    // Save the binary image
    char output_path[1024];
    sprintf(output_path, "output_folder/omp_otsu%s", filename);
    printf("Saving image to path: %s\n", output_path);
    if (!stbi_write_png(output_path, width, height, 1, binary_img, width)) {
        fprintf(stderr, "Error writing image %s\n", output_path);
    }

    // Clean up
    if (channels != 1) free(gray_img);
    free(binary_img);
}