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
char *map_find(char * key);
int map_insert(char *key, char *value);

void print_map_content();

#endif // MAP
