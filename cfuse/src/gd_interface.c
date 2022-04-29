#include <curl/curl.h>
#include "gd_interface.h"

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

int gdi_init() {
    return read_config("../config.txt");
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
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

    return res == CURLE_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

int update_file(char *id, char* data) {
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

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);

}

int main(void)
{
    if (gdi_init() == EXIT_FAILURE)
        return EXIT_FAILURE;

    // get_file_list();
    FILE *file = fopen("out.txt", "w");
    get_file_by_id("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);
    
    return 0;
}