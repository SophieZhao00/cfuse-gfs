#include "map.h"

struct item idmap[MAX_MAPSIZE];
int map_size;

void map_init() {
    map_size = 0;
}

char *map_find(char * key) {
    for (int i = 0; i < map_size; ++i) {
        if (strcmp(idmap[i].key, key) == 0) {
            return idmap[i].value;
        }
    }
    return NULL;
}

int map_insert(char *key, char *value) {
    char * old_value = map_find(key);
    if (old_value != NULL) {
        strcpy(old_value, value);
        return EXIT_SUCCESS;
    }
    strcpy(idmap[map_size].key, key);
    strcpy(idmap[map_size].value, value);
    map_size++;
    if (map_size >= MAX_MAPSIZE) {
        fprintf(stderr, "Map is full\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void print_map_content() {
    printf("*** Start to print map content:\n");
    for (int i = 0; i < map_size; ++i) {
        printf("Key: %s\t Value: %s\n", idmap[i].key, idmap[i].value);
    }
    printf("Print map content ends ***\n");
}