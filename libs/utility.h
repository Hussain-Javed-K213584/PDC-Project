#ifndef UTILITY_H
#define UTILITY_H

#include <sys/stat.h>
#include <sys/types.h>

// Create a directory if not already created
void create_output_directory(const char *output_dir_name);

#endif