#ifndef MAP
#define MAP

#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define MAX_MAPSIZE 100
// const char CACHE_FILE[] = "file_list_cache.txt";

struct item {
    char key[50];
    char value[50];
    off_t size;
};

struct item idmap[MAX_MAPSIZE];

void map_init();
int map_find(const char * key, char *value);
off_t map_find_size(const char *key);
int map_insert(const char *key, const char *value, off_t size);
int map_exists(const char* key);
int generate_map();

void print_map_content();


int map_size;

#endif // MAP
