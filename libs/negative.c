#include "negative.h"

// Converts an image to negative
void negative_serial(unsigned char *input_image, unsigned char *output_image, int width, int height, int channels) 
{
    size_t img_size = width * height * channels;
    for (size_t i = 0; i < img_size; i++) {
        output_image[i] = 255 - input_image[i];
    }
}

void negative_omp(unsigned char *input_image, unsigned char *output_image, int width, int height, int channels)
{
    size_t img_size = width * height * channels;
    #pragma omp parallel for
    for (size_t i = 0; i < img_size; i++) {
        output_image[i] = 255 - input_image[i];
    }
}