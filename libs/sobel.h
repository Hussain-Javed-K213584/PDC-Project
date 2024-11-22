#ifndef SOBEL_H
#define SOBEL_H

#include <stddef.h>

// Function to perform Sobel edge detection on a grayscale image
void sobel_filter(const unsigned char *input_image, unsigned char *output_image,
                  int width, int height);

#endif // SOBEL_H
