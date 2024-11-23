#ifndef OTSU_H
#define OTSU_H

#include "image.h"
#include <stddef.h>

int compute_otsu_threshold(const unsigned char *gray_image, int width, int height);
void apply_threshold(const unsigned char *gray_image, unsigned char *binary_image,
                     int width, int height, int threshold);

int compute_otsu_threshold_omp(const unsigned char *gray_image, int width, int height);
void apply_threshold_omp(const unsigned char *gray_image, unsigned char *binary_image,
                     int width, int height, int threshold);

void otsu_serial(unsigned char *img, const char *filename, int user_threshold, int width, int height, int channels);

void otsu_omp(unsigned char *img, const char *filename, int user_threshold, int width, int height, int channels);

#endif
