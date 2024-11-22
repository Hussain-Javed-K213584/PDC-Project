#ifndef SOBEL_H
#define SOBEL_H

#include "image.h"
#include <stddef.h>
#include <omp.h>
#include "utility.h"

// Function to perform Sobel edge detection on a grayscale image
void sobel_filter(const unsigned char *input_image, unsigned char *output_image,
                  int width, int height);

void sobel_filter_omp(const unsigned char *input_image, unsigned char *output_image,
                  int width, int height);

void sobel_filter_hybrid(const char *filename, const char *input_folder, unsigned char *output_image);

#endif // SOBEL_H
