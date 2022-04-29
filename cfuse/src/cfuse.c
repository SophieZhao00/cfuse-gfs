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

#define PATH_MAX 256
#define CMD_MAX 256

// Command line options
static struct options {
    const char* hostname;
    const char* cache;
} options;

// #define OPTION(t, p)                           \
//     { t, offsetof(struct options, p), 1 }
// static const struct fuse_opt option_spec[] = {
//     OPTION("--host=%s", hostname),
//     OPTION("--cache=%s", cache),
//     FUSE_OPT_END
// };

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

int is_valid(const char* path) {
    // Check if the path is valid
    return 1;
}

// convert path to real path on local file system
static void cgfs_realpath(char fpath[PATH_MAX], const char *path)
{
    // strcpy(fpath, options.cache);
    strncat(fpath, path, PATH_MAX);
}

static int cgfs_getattr(const char *path, struct stat *stbuf,
       struct fuse_file_info *fi)
{
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
    memset(stbuf, 0, sizeof(stbuf));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } 
    if (is_valid(path)) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        // TODO: get file's megadata and there's a field called size.
        // stbuf->st_size = get_size(path);
        return 0;
    }
    return -ENOENT;
}

static int cgfs_open(const char *path, struct fuse_file_info *fi)
{
    // get the real path on the local machine
    char fpath[PATH_MAX];
    cgfs_realpath(fpath, path);
    // TODO: Download the file from Google drive to real path

    int fd = open(fpath, fi->flags);
    fi->fh = fd;
    if (fd < 0)
        return -errno;
    else
        return 0;
}

static int cgfs_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
    return pread(fi->fh, buf, size, offset);  
}

static int cgfs_write(const char *path, const char *buf, size_t size, off_t offset,
       struct fuse_file_info *fi)
{
    int res = pwrite(fi->fh, buf, size, offset);
    // scp_write(path);
    return res;
}

static int cgfs_fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
{
    int res;
    if (isdatasync) {
        res = fdatasync(fi->fh);
    } else {
        res = fsync(fi->fh);
    }
    // scp_write(path);
    // TODO: Update the file back to the Google Drive
    return res;
}

static int cgfs_release(const char* path, struct fuse_file_info *fi)
{
    // TODO: Update the file back to the Google Drive (Is this needed??)
    return close(fi->fh);
}

static const struct fuse_operations cgfs_oper = {
    .getattr	= cgfs_getattr,
    .open		= cgfs_open,
    .read		= cgfs_read,
    .write = cgfs_write,
    .fsync = cgfs_fsync,
    .release = cgfs_release,
};

int main(int argc, char *argv[]) {
    int ret;

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    // Set defaults
    // options.hostname = strdup("xuefeiz@auriga.cs.utexas.edu:/tmp/xuefeiz/root");
    // options.cache = strdup("/tmp/xuefeiz");

    // Parse options
    // if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    //     fprintf(stderr, "fail to parse options\n");
    //     return 1;
    // }

    ret = fuse_main(argc, argv, &cgfs_oper, NULL);

    // fuse_opt_free_args(&args);

    return ret;
}
