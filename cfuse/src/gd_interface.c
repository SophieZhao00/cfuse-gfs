#include "gd_interface.h"

#include <curl/curl.h>
#include "map.h"

static char access_token[200];
static char api_key[50];
char root_id[50];

static int read_config(char* path) {
    // open file
    FILE *file = fopen(path, "r");
    if (!file) {
        perror(path);
        return EXIT_FAILURE;
    }

    // parse config
    if (!fgets(access_token, 200, file)) {
        perror("Failed to get access token\n.");
        return EXIT_FAILURE;
    }
    if (!fgets(api_key, 50, file)) {
        perror("Failed to get API key\n.");
        return EXIT_FAILURE;
    }
    if (!fgets(root_id, 50, file)) {
        perror("Failed to get API key\n.");
        return EXIT_FAILURE;
    }

    // trim lines
    if (access_token[strlen(access_token) - 1] == '\n')
        access_token[strlen(access_token) - 1] = 0;
    if (api_key[strlen(api_key) - 1] == '\n')
        api_key[strlen(api_key) - 1] = 0;
    if (root_id[strlen(root_id) - 1] == '\n')
        root_id[strlen(root_id) - 1] = 0;

    // printf("access_token: %s\n", access_token);
    // printf("api_key: %s\n", api_key);
    // printf("root_id: %s\n", root_id);

    // close file
    if (fclose(file)) {
        perror(path);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static size_t file_list_writefunc(void *data_in, size_t size, size_t nmemb, void *data_out) {
    char * data = (char *) data_in;
    char * files = strstr(data, "\"files\": [");
    files += strlen("\"files\": [\n");
    char *name, *id;
    id = strstr(files, "\"id\": ");
    while (id != NULL) {
        name = strstr(files, "\"name\": ");
        if (name == NULL) {
            fprintf(stderr, "Id exists but name doesn't\n");
            return EXIT_FAILURE;
        }
        char * id_end = strchr(id, ',');
        char id_content[50];
        id += 7;
        id_end -= 1;
        if (id_end-id >= 50) {
            fprintf(stderr, "id has more than 50 characters\n");
            return EXIT_FAILURE;
        }
        strncpy(id_content, id, id_end - id);
        id_content[id_end - id] = '\0';
        // printf("The id is: [%s]\n", id_content);

        char * name_end = strchr(name, ',');
        char name_content[50];
        name += 9;
        name_end -= 1;
        if (name_end-name >= 50) {
            fprintf(stderr, "Name has more than 50 characters\n");
            return EXIT_FAILURE;
        }
        strncpy(name_content, name, name_end - name);
        name_content[name_end - name] = '\0';
        // printf("The name is: [%s]\n", name_content);

        int ret_v = map_insert(name_content, id_content);
        if (ret_v == EXIT_FAILURE) {
            fprintf(stderr, "Fail to insert the name-id pair into the map\n");
            return EXIT_FAILURE;
        }

        id = strstr(id, "\"id\": ");
        char * max_position = name > id ? name : id;
        files = strchr(max_position, '\n');
    }
    return size*nmemb;
}

int gdi_init() {
    map_init();
    return read_config("cfuse/config.txt");
}

int get_file_list() {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
        return EXIT_FAILURE;

    struct curl_slist *chunk = NULL;
    char header[250];
    strcpy(header, "Authorization: Bearer ");
    strcat(header, access_token);
    chunk = curl_slist_append(chunk, header);
    chunk = curl_slist_append(chunk, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    // GET file list
    char url[200];
    strcpy(url, "https://www.googleapis.com/drive/v3/files?q=%27");
    strcat(url, root_id);
    strcat(url, "%27%20in%20parents&key=");
    strcat(url, api_key);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_list_writefunc);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return res == CURLE_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

int get_file_by_id(char *id, FILE *file) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
        return EXIT_FAILURE;

    struct curl_slist *chunk = NULL;
    char header[250];
    strcpy(header, "Authorization: Bearer ");
    strcat(header, access_token);
    chunk = curl_slist_append(chunk, header);
    chunk = curl_slist_append(chunk, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    // GET file by id
    char url[200];
    strcpy(url, "https://www.googleapis.com/drive/v3/files/");
    strcat(url, id);
    strcat(url, "?alt=media&key=");
    strcat(url, api_key);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return res == CURLE_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

int update_file(char *id, FILE *file) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
        return EXIT_FAILURE;
    
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");

    struct curl_slist *chunk = NULL;
    char header[250];
    strcpy(header, "Authorization: Bearer ");
    strcat(header, access_token);
    chunk = curl_slist_append(chunk, header);
    chunk = curl_slist_append(chunk, "Accept: application/json");
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    // Update (PATCH) file
    char url[200];
    strcpy(url, "https://www.googleapis.com/upload/drive/v3/files/");
    strcat(url, id);
    strcat(url, "?uploadType=media&key=");
    strcat(url, api_key);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_READDATA, file);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return res == CURLE_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

// int main(void)
// {
//     if (gdi_init() == EXIT_FAILURE)
//         return EXIT_FAILURE;

//     // get_file_list();
//     // print_map_content();

//     FILE *file = fopen("out.txt", "w");
//     get_file_by_id("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);

//     // FILE *file = fopen("out.txt", "r");
//     // update_file("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);
    
//     return 0;
// }
