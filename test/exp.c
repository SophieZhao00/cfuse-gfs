#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE (1024 * 1024)

const int NUM_SIZES = 7;
const int NUM_FILES = 10;

char buffer[BUFFER_SIZE];

char path[] = "/home/xuefeiz/cfuse-gfs/mount_point/f";
char size_string[7][6] = {"1KB", "8KB", "64KB", "512KB", "4MB", "32MB", "256MB"};

int write_test(int size) {
    int fd;
    int size_left;
    int write_ret = 0;
    int ret;
    int flag = 1;
    char file_name[40];
    for (int i = 0; i < NUM_FILES; ++i) {
        strcpy(file_name, path);
        file_name[strlen(path)] = '0' + i;
        file_name[strlen(path) + 1] = '\0';
        fd = open(file_name, O_WRONLY);
        // printf("file %s open: %d\n", file_name, fd);
        size_left = size;
        write_ret = 0;
        while (size_left > BUFFER_SIZE) {
            size_left -= BUFFER_SIZE;
            ret = write(fd, buffer, BUFFER_SIZE);
            write_ret += ret;
            if (ret == -1) flag = 0;
        }
        if (size_left > 0) {
            ret = write(fd, buffer, size_left);
            write_ret += ret;
            if (ret == -1) flag = 0;
        }
        close(fd);
        printf("file write: %d %s\n", write_ret, file_name);
    }
    return flag;
}

int read_test(size_t size) {
    int fd;
    size_t size_left;
    int ret;
    int flag = 1;
    int read_ret = 0;
    char file_name[40];
    for (int i = 0; i < NUM_FILES; ++i) {
        strcpy(file_name, path);
        file_name[strlen(path)] = '0' + i;
        file_name[strlen(path) + 1] = '\0';
        fd = open(file_name, O_RDONLY);
        // printf("file %s open: %d\n", file_name, fd);
        size_left = size;
        read_ret = 0;
        while (size_left > BUFFER_SIZE) {
            size_left -= BUFFER_SIZE;
            ret = read(fd, buffer, BUFFER_SIZE);
            read_ret += ret;
            if (ret == -1 || ret == 0) flag = 0;
        }
        if (size_left > 0) {
            ret = read(fd, buffer, size_left);
            read_ret += ret;
            if (ret == -1 || ret == 0) flag = 0;
        }
        close(fd);
        printf("file read: %d %s\n", read_ret, file_name);
    }
    return flag;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: exp <index>\n");
        return 0;
    }
    
    const int start = 1024;
    size_t sizes[NUM_SIZES];
    int flag;
    sizes[0] = start;
    for (int i = 1; i < NUM_SIZES; ++i) {
        sizes[i] = 8 * sizes[i - 1]; 
    }

    // init buffer
    // int *int_buffer = (int *)buffer;
    // for (int i = 0; i < BUFFER_SIZE / 4; ++i) {
    //     // int_buffer[i] = BUFFER_SIZE / 4 + 1 - i;
    //     int_buffer[i] = i + 1;
    // }
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = 'a';
    }

    clock_t start_time;
    clock_t end_time;

    int ind = 0;
    ind = argv[1][0] - '0';

    FILE* f = fopen("cfuse_result.txt", "a");
    fprintf(f, " size \t write \t read\n");
    // for (int ind = 0; ind < NUM_SIZES; ++ind) {
    fprintf(f, "%6s\t", size_string[ind]);
    fflush(f);

    //write tests
    printf("\nWrite test with size: %5s %ld\n", size_string[ind], sizes[ind]);
    start_time = clock();
    flag = write_test(sizes[ind]);
    end_time = clock();
    printf("Flag: %d\tTime: %ld\n", flag, end_time - start_time);
    fprintf(f, "%7ld\t", end_time - start_time);
    fflush(f);

    //read tests
    printf("\nRead test with size: %5s %ld\n", size_string[ind], sizes[ind]);
    start_time = clock();
    flag = read_test(sizes[ind]);
    end_time = clock();
    printf("Flag: %d\tTime: %ld\n", flag, end_time - start_time);
    fprintf(f, "%7ld\n", end_time - start_time);
    fflush(f);
    // }

    // results
    printf("Experiment ends.\n");
    
    return 0;
}
