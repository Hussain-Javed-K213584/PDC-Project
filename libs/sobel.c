#include "sobel.h"
#include <stdlib.h>
#include <math.h>

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