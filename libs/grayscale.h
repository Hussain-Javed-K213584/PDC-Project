#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mpi/mpi.h"
#include <string.h>
#include "utility.h"

// Serial Grayscale Function
void grayscale_serial(unsigned char *buffer, unsigned char *output, int width, int height, int channels, const char *output_folder, const char *original_file);

// OpenMP Grayscale Function
void grayscale_openmp(unsigned char *buffer, unsigned char *output, int width, int height, int channels, const char* output_folder, const char *original_file);

void save_image(const char *output_folder, const char *original_file, unsigned char *output, int width, int height, int channels);

void process_image_mpi(const char *filename, const char *input_folder);
 
#endif // GRAYSCALE_H