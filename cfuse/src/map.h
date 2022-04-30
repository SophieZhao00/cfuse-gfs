#ifndef MAP
#define MAP

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_MAPSIZE 100

struct item {
    char key[50];
    char value[50];
};

void map_init();
int map_find(const char * key, char *value);
int map_insert(const char *key, const char *value);
int map_exists(const char* key);

void print_map_content();

#endif // MAP
