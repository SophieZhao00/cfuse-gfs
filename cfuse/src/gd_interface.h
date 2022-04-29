#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int gdi_init();

int get_file_list();

int get_file_by_id(char *id, FILE *file);

int update_file(char *id, char* data);