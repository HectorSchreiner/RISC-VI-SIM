#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

/// @brief `FileHandler` is a struct that will do all file operations on the input file/files.
/// If you want to add an operation to this file, create a function declaration, and add it as a function pointer
/// this way we can simulate methods.
struct FileHandler
{
    int* (*read_file)(struct FileHandler *self, char *filename, int *size);
    int (*write_file)(char name[], int arr[], int n);
};

struct FileHandler* create_filehandler();

int* read_file(struct FileHandler* fh, char *filename, int *size);
int write_file(char name[], int arr[], int n);
#endif
