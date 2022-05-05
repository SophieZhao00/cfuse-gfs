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
    // fprintf(log_fd, "cgfs_realpath: %s\n", path);
    // fflush(log_fd);
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
    // fprintf(log_fd, "cgfs_getattr: %s\n", path);
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

    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);

    // if (strcmp(path, "/.xdg-volume-info") == 0 ||
    // strcmp(path, "/autorun.inf") == 0 ||
    // strcmp(path, "/.git") == 0) {
        // char fpath[PATH_MAX];
        // cgfs_realpath(fpath, path);
        // fprintf(log_fd, "lstat = %d\n", lstat(fpath, stbuf));
        // fflush(log_fd);
    //     return -1;
    // }
    
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        if (get_file_list() == EXIT_FAILURE) {
            // fprintf(log_fd, "fail to get file list\n");
            // fflush(log_fd);
        }
        return 0;
    } else if (is_valid(fname) == EXIT_SUCCESS) {
        // fprintf(log_fd, "File %s in file list\n", path);
        // fflush(log_fd);
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        // TODO: get file's metadata and there's a field called size.
        // stbuf->st_size = get_size(path);
        stbuf->st_size = map_find_size(fname);
        return 0;
    }
    return -1;
}

static int cgfs_open(const char *path, struct fuse_file_info *fi)
{
    // fprintf(log_fd, "cgfs_open: %s\n", path);
    // fflush(log_fd);
    
    // if (get_file_list() == EXIT_FAILURE) {
    //     fprintf(log_fd, "fail to get file list\n");
    //     fflush(log_fd);
    // }

    // get the real path on the local machine
    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);
    char fpath[PATH_MAX];
    cgfs_realpath(fpath, path);
    // TODO: Download the file from Google drive to real path
    // int fd = fileno(cache_file);
    // fprintf(log_fd, "fpath: %s\n", fpath);
    // fflush(log_fd);
    // int fd = open(fpath, O_WRONLY);
    // char mode[5];
    // flags_into_str(fi->flags, mode);
    // fprintf(log_fd, "mode=%s\n", mode);
    // fflush(log_fd);
    // FILE* cache_file = fdopen(fd, "w");
    FILE* cache_file = fopen(fpath, "w");
    char id[50];
    map_find(fname, id);
    // fprintf(log_fd, "Name=%s\tID=%s\n", fname, id);
    // fflush(log_fd);
    // get_file_by_id(id, cache_file);
    if (get_file_by_id(id, cache_file) == EXIT_FAILURE) {
        // fprintf(log_fd, "fail to get file\n");
        // fflush(log_fd);
    }
    
    fclose(cache_file);
    int fd = open(fpath, fi->flags);
    // fprintf(log_fd, "flag: %s %d\n", fpath, fi->flags);
    // fflush(log_fd);

    // TODO: free(cache_file);

    fi->fh = fd;
    // fprintf(log_fd, "exit cgfs_open: %s \tfd: %d\n", path, fd);
    // fflush(log_fd);
    if (fd < 0)
        return -errno;
    else
        return 0;
}

static int cgfs_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
    // fprintf(log_fd, "Read from file %s with size %ld and offset %ld\n", path, size, offset);
    // fprintf(log_fd, "%ld %d\n", fi->fh, fi->flags);
    // fflush(log_fd);
    // int ret = pread(fi->fh, buf, size, offset);
    // fprintf(log_fd, "Read result: %d %s\n", ret, buf);
    // fflush(log_fd);
    return pread(fi->fh, buf, size, offset);
}

static int cgfs_write(const char *path, const char *buf, size_t size, off_t offset,
       struct fuse_file_info *fi)
{
    // fprintf(log_fd, "Write to file %s\n", path);
    // fflush(log_fd);
    int res = pwrite(fi->fh, buf, size, offset);
    // scp_write(path);
    return res;
}

// static int cgfs_fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
// {
//     // fprintf(log_fd, "synchronize file %s\n", path);
//     // fflush(log_fd);
//     int res;
//     if (isdatasync) {
//         res = fdatasync(fi->fh);
//     } else {
//         res = fsync(fi->fh);
//     }
//     // Update the file back to the Google Drive
//     char fname[PATH_MAX];
//     cgfs_pathtoname(fname, path);
//     FILE* cache_file = fdopen(fi->fh, "r");
//     char id[50];
//     map_find(fname, id);
//     update_file(id, cache_file);

//     return res;
// }

static int cgfs_release(const char *path, struct fuse_file_info *fi)
{
    // fprintf(log_fd, "cgfs_release %s\n", path);
    // fflush(log_fd);
    // TODO: Update the file back to the Google Drive (Is this needed??)
    int ret = close(fi->fh);
    if (fi->flags & O_RDONLY) return ret;
    
    char fname[PATH_MAX];
    cgfs_pathtoname(fname, path);

    char fpath[PATH_MAX];
    cgfs_realpath(fpath, path);
    FILE* cache_file = fopen(fpath, "r");

    // fprintf(log_fd, "*** Start to print map content:\n");
    // for (int i = 0; i < map_size; ++i) {
    //     fprintf(log_fd, "Key: %s\t Value: %s\n", idmap[i].key, idmap[i].value);
    // }
    // fprintf(log_fd, "Print map content ends ***\n");
    // fflush(log_fd);

    char id[50];
    if (map_find(fname, id) == EXIT_FAILURE) {
        // fprintf(log_fd, "fname %s isn't in the map\n", fname);
        // fflush(log_fd);
    }
    update_file(id, cache_file);
    fclose(cache_file);
    return ret;
}

static const struct fuse_operations cgfs_oper = {
    .getattr	= cgfs_getattr,
    .open		= cgfs_open,
    .read		= cgfs_read,
    .write      = cgfs_write,
    // .fsync      = cgfs_fsync,
    .release    = cgfs_release,
};

int main(int argc, char *argv[]) {
    int ret;

    // log_fd = fopen("log.txt", "w");
    // fprintf(log_fd, "Log start\n");
    // fflush(log_fd);

    getcwd(cache, PATH_MAX);
    strcat(cache, "/cache");

    if (gdi_init() == EXIT_FAILURE)
        return EXIT_FAILURE;

    ret = fuse_main(argc, argv, &cgfs_oper, NULL);

    // if (get_file_list() == EXIT_FAILURE) {
    //     printf("fail to get file list\n");
    // } else print_map_content();

    // FILE *file = fopen("out.txt", "w");
    // get_file_by_id("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);

    // FILE *file = fopen("out.txt", "r");
    // update_file("16X-Bc4N8Gv2Z_PzArFbdEpDTch3MkPZR", file);

    return ret;
}
