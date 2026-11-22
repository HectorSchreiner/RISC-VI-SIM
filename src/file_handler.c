#include "file_handler.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

struct FileHandler *create_filehandler() {
  struct FileHandler *fh = malloc((sizeof(struct FileHandler)));
  fh->read_file = read_file;
  fh->write_file =write_file;
  return fh;
}

int *read_file(struct FileHandler *fh,char *filename, int *size) {
  //char filename[] = "tests/addlarge.bin";

  FILE *fptr = fopen(filename, "rb");
  if (fptr == NULL) {
    printf("Error! opening file\n");
    return NULL;
  }

  struct stat st;
  if (stat(filename, &st) != 0) {
    fclose(fptr);
    return NULL;
  }
  *size = st.st_size; // size in BYTES

  int *instructions = malloc(st.st_size);
  if (!instructions) {
    fclose(fptr);
    return NULL;
  }

  fread(instructions, sizeof(int), st.st_size / sizeof(int), fptr);
  fclose(fptr);

  return instructions;
}
int write_file(char name[], int arr[], int n){

    FILE *fptr = fopen(name, "wb");

    fwrite(arr, sizeof(int), n, fptr);

    fclose(fptr);
    return 0;
}