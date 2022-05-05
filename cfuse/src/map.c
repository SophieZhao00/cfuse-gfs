#include "map.h"


void map_init() {
    map_size = 0;
}

int map_find(const char * key, char *value) {
    for (int i = 0; i < map_size; ++i) {
        if (strcmp(idmap[i].key, key) == 0) {
            strcpy(value, idmap[i].value);
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

off_t map_find_size(const char *key) {
    for (int i = 0; i < map_size; ++i) {
        if (strcmp(idmap[i].key, key) == 0) {
            return idmap[i].size;
        }
    }
    return -1;
}

int map_insert(const char *key, const char *value, off_t size) {
    for (int i = 0; i < map_size; ++i) {
        if (strcmp(idmap[i].key, key) == 0) {
            strcpy(idmap[i].value, value);
            idmap[i].size = size;
            return EXIT_SUCCESS;
        }
    }
    strcpy(idmap[map_size].key, key);
    strcpy(idmap[map_size].value, value);
    idmap[map_size].size = size;
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
        printf("Key: %5s\t Value: %s\t Size:%ld\n", idmap[i].key, idmap[i].value, idmap[i].size);
    }
    printf("Print map content ends ***\n");
}

int map_exists(const char* key) {
    for (int i = 0; i < map_size; ++i) {
        if (strcmp(key, idmap[i].key) == 0)
            return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int generate_map() {
    FILE* file = fopen("/home/xuefeiz/cfuse-gfs/file_list_cache.txt", "r");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    char *line = NULL;
    size_t len = 0;
    char id[50];
    char name[50];
    char size[50];
    char *begin, *end;
    off_t size_num = 0;
    char * useless;
    while (getline(&line, &len, file) != -1) {
        begin = strstr(line, "\"id\":");
        if (begin != NULL) {
            end = strchr(line, ',');
            end -= 1;
            begin += 7;
            strncpy(id, begin, end - begin);
            id[end - begin] = '\0';
            begin = NULL;
            end = NULL;
        }
        begin = strstr(line, "\"name\":");
        if (begin != NULL) {
            end = strchr(line, ',');
            end -= 1;
            begin += 9;
            strncpy(name, begin, end - begin);
            name[end - begin] = '\0';
            begin = NULL;
            end = NULL;

        }
        
        begin = strstr(line, "\"size\":");
        if (begin != NULL) {
            end = strchr(line, ',');
            end -= 1;
            begin += 9;
            strncpy(size, begin, strlen(begin) - 2);
            size[strlen(begin) - 2] = '\0';
            printf("size: %s\n", size);
            size_num = strtol(size, &useless, 10);
            begin = NULL;
            end = NULL;
     
            int ret_v = map_insert(name, id, size_num);
            if (ret_v == EXIT_FAILURE) {
                fprintf(stderr, "Fail to insert the name-id pair into the map\n");
                return EXIT_FAILURE;
            }
        }
    }
    free(line);
    fclose(file);
    return EXIT_SUCCESS;
//     char * data = (char *) data_in;
//     char * files = strstr(data, "\"files\": [");
//     files += strlen("\"files\": [\n");
//     char *name, *id;
//     id = strstr(files, "\"id\": ");
//     while (id != NULL) {
//         name = strstr(files, "\"name\": ");
//         if (name == NULL) {
//             fprintf(stderr, "Id exists but name doesn't\n");
//             return EXIT_FAILURE;
//         }
//         char * id_end = strchr(id, ',');
//         char id_content[50];
//         id += 7;
//         id_end -= 1;
//         if (id_end-id >= 50) {
//             fprintf(stderr, "id has more than 50 characters\n");
//             return EXIT_FAILURE;
//         }
//         strncpy(id_content, id, id_end - id);
//         id_content[id_end - id] = '\0';
//         // printf("The id is: [%s]\n", id_content);

//         char * name_end = strchr(name, ',');
//         char name_content[50];
//         name += 9;
//         name_end -= 1;
//         if (name_end-name >= 50) {
//             fprintf(stderr, "Name has more than 50 characters\n");
//             return EXIT_FAILURE;
//         }
//         strncpy(name_content, name, name_end - name);
//         name_content[name_end - name] = '\0';
//         // printf("The name is: [%s]\n", name_content);

//         int ret_v = map_insert(name_content, id_content);
//         if (ret_v == EXIT_FAILURE) {
//             fprintf(stderr, "Fail to insert the name-id pair into the map\n");
//             return EXIT_FAILURE;
//         }

//         id = strstr(id, "\"id\": ");
//         char * max_position = name > id ? name : id;
//         files = strchr(max_position, '\n');
//     }
//     return size*nmemb;
}