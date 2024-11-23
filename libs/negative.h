#ifndef NEGATIVE_H
#define NEGATIVE_H

#include <stddef.h>

void negative_serial(unsigned char *input_image, unsigned char *output_image, int width, int height, int channels);

void negative_omp(unsigned char *input_image, unsigned char *output_image, int width, int height, int channels);

#endif