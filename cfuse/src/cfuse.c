#define FUSE_USE_VERSION 31
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gd_interface.h"
#include "map.h"

#define PATH_MAX 256
#define CMD_MAX 256

char cache[PATH_MAX];

FILE* log_fd;

/*
// Command line options
static struct options {
    const char* mount_point;
    const char* cache;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
    OPTION("--mount_point=%s", mount_point),
    OPTION("--cache=%s", cache),
    FUSE_OPT_END
};
*/

// scp files from host to cache
// void scp_read(const char* path) {
//     // TODO: does not support sub directoreis
//     char cmd[CMD_MAX] = "scp -r ";
//     strncat(cmd, options.hostname, CMD_MAX);
//     strncat(cmd, path, CMD_MAX);
//     strcat(cmd, " ");
//     strncat(cmd, options.cache, CMD_MAX);

//     system(cmd);
// }

// scp files from cache to host
// void scp_write(const char* path) {
//     // TODO: does not support sub directoreis
//     char cmd[CMD_MAX] = "scp -r ";
//     strncat(cmd, options.cache, CMD_MAX);
//     strncat(cmd, path, CMD_MAX);
//     strcat(cmd, " ");
//     strncat(cmd, options.hostname, CMD_MAX);

//     system(cmd);
// }

static int is_valid(const char* path) {
    // Check if the path is valid
    return map_exists(path);
}

int flags_into_str(int flags, char* str) {
    // Only considering O_RDONLY, O_WRONLY, and O_RDWR
    if (flags & O_RDONLY) {
        strcpy(str, "r");
    } else if (flags & O_WRONLY) {
        strcpy(str, "w");
    } else {
        strcpy(str, "r+");
    }
    return EXIT_SUCCESS;
}

// convert path to real path on local file system
static void cgfs_realpath(char fpath[PATH_MAX], const char *path)
{
    fprintf(log_fd, "cgfs_realpath: %s\n", path);
    fflush(log_fd);
    strcpy(fpath, cache);
    strncat(fpath, path, PATH_MAX);
}

static void cgfs_pathtoname(char fname[PATH_MAX], const char *path) {
    if (strcmp(path, "/") == 0) {
        strcpy(fname, path);
        return;
    }
    char * st = strrchr(path, '/');
    st ++;
    strcpy(fname, st);
}

static int cgfs_getattr(const char *path, struct stat *stbuf,
       struct fuse_file_info *fi)
{
    fprintf(log_fd, "cgfs_getattr: %s\n", path);
    // get the real path on the local machine
    // char fpath[PATH_MAX];
    // cgfs_realpath(fpath, path);

    // // read files
    // if (strcmp(path, "/") == 0) {
    //     scp_read("/*");
    // } else {
    //     scp_read(path);
    // }

    // memset(stbuf, 0, sizeof(struct stat));
    // return lstat(fpath, stbuf);
    memset(stbuf, 0, sizeof(struct stat));

    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);

    char fpath[PATH_MAX];
    cgfs_realpath(fpath, path);

    fflush(stdout);
    fflush(stderr);

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        if (get_file_list() == EXIT_FAILURE) {
            fprintf(log_fd, "fail to get file list\n");
            fflush(log_fd);
        }
        return 0;
    } 
    if (is_valid(fname) == EXIT_SUCCESS) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        // TODO: get file's metadata and there's a field called size.
        // stbuf->st_size = get_size(path);
        return 0;
    }
    return lstat(fpath, stbuf);
}

static int cgfs_open(const char *path, struct fuse_file_info *fi)
{
    fprintf(log_fd, "cgfs_open: %s\n", path);
    fflush(log_fd);
    // get the real path on the local machine
    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);
    char fpath[PATH_MAX];
    cgfs_realpath(fpath, path);
    // TODO: Download the file from Google drive to real path
    // int fd = fileno(cache_file);
    fprintf(log_fd, "fpath: %s\n", fpath);
    fflush(log_fd);
    // int fd = open(fpath, O_WRONLY);
    // char mode[5];
    // flags_into_str(fi->flags, mode);
    // fprintf(log_fd, "mode=%s\n", mode);
    // fflush(log_fd);
    // FILE* cache_file = fdopen(fd, "w");
    FILE* cache_file = fopen(fpath, "w");
    char id[50];
    map_find(fname, id);
    fprintf(log_fd, "Name=%s\tID=%s\n", fname, id);
    fflush(log_fd);
    // get_file_by_id(id, cache_file);
    if (get_file_by_id(id, cache_file) == EXIT_FAILURE) {
        fprintf(log_fd, "fail to get file\n");
        fflush(log_fd);
    }
    fprintf(log_fd, "before flush: %s\n", path);
    fflush(log_fd);
    fflush(cache_file);
    fprintf(log_fd, "after flush: %s\n", path);
    fflush(log_fd);
    
    fclose(cache_file);
    int fd = open(fpath, fi->flags);

    // TODO: free(cache_file);

    fi->fh = fd;
    fprintf(log_fd, "exit cgfs_open: %s\n", path);
    fflush(log_fd);
    if (fd < 0)
        return -errno;
    else
        return 0;
}

static int cgfs_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
    fprintf(log_fd, "Read from file %s\n", path);
    fflush(log_fd);
    return pread(fi->fh, buf, size, offset);
}

static int cgfs_write(const char *path, const char *buf, size_t size, off_t offset,
       struct fuse_file_info *fi)
{
    fprintf(log_fd, "Write to file %s\n", path);
    fflush(log_fd);
    int res = pwrite(fi->fh, buf, size, offset);
    // scp_write(path);
    return res;
}

static int cgfs_fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
{
    fprintf(log_fd, "synchronize file %s\n", path);
    fflush(log_fd);
    int res;
    if (isdatasync) {
        res = fdatasync(fi->fh);
    } else {
        res = fsync(fi->fh);
    }
    // scp_write(path);
    // TODO: Update the file back to the Google Drive
    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);
    // char mode[5];
    // flags_into_str(fi->flags, mode);
    FILE* cache_file = fdopen(fi->fh, "r");
    char id[50];
    map_find(fname, id);
    update_file(id, cache_file);

    return res;
}

static int cgfs_release(const char *path, struct fuse_file_info *fi)
{
    fprintf(log_fd, "cgfs_release %s\n", path);
    fflush(log_fd);
    // TODO: Update the file back to the Google Drive (Is this needed??)
    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);
    // char mode[5];
    // flags_into_str(fi->flags, mode);
    FILE* cache_file = fdopen(fi->fh, "r");
    char id[50];
    map_find(fname, id);
    update_file(id, cache_file);

    return close(fi->fh);
}

static const struct fuse_operations cgfs_oper = {
    .getattr	= cgfs_getattr,
    .open		= cgfs_open,
    .read		= cgfs_read,
    .write      = cgfs_write,
    .fsync      = cgfs_fsync,
    .release    = cgfs_release,
};

int main(int argc, char *argv[]) {
    int ret;

    log_fd = fopen("log.txt", "w");
    fprintf(log_fd, "Log start\n");
    fflush(log_fd);
    // struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    // Set defaults
    // options.mount_point = strdup("./mount");
    // options.cache = strdup("./cache/");

    // Parse options
    // if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    //     fprintf(stderr, "fail to parse options\n");
    //     return 1;
    // }

    getcwd(cache, PATH_MAX);
    strcat(cache, "/cache");
    // printf("Current working directory: %s\n", cache);

    // printf("mount_point: %s\n", options.mount_point);
    // printf("cache: %s\n", options.cache);

    if (gdi_init() == EXIT_FAILURE)
        return EXIT_FAILURE;

    ret = fuse_main(argc, argv, &cgfs_oper, NULL);

    // fuse_opt_free_args(&args);

    // FILE *file = fopen("/home/xuefeiz/cfuse-gfs/cache/foo", "r+");
    // get_file_by_id("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);

    return ret;
}
