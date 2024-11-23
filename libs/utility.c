#include "utility.h"

// Courtesy of stack overflow: https://stackoverflow.com/a/7430262
void create_output_directory(const char *output_dir_name) {
    
    struct stat st = {0};
    if (stat(output_dir_name, &st) == -1) {
        mkdir(output_dir_name, 0700);
    }

}