/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2005  Miklos Szeredi <miklos@szeredi.hu>
    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

static int files = 1000;
static int files_start = 40;

static char blocks[1000][1000];
static const char *lsf_str = "Hello World!\n";
static const char *lsf_path = "/hello";


static int get_block_num(const char* path)
{
    char * filename = malloc(strlen(path));
    memcpy(filename, path+1, strlen(path));
    
    int i = 0;
    char temp[4];
    for (i=files_start; i<files;i++){
        sprintf(temp, "%d", i);
        if (strcmp(filename,temp) == 0){
            printf("file %d exist\n",i);
            return i;
        }
    }
    return -1;
}


static int lsf_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
    else if(strcmp(path, lsf_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(lsf_str);
    }
    else
        res = -ENOENT;

    return res;
}

static int lsf_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if(strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, lsf_path + 1, NULL, 0);

    return 0;
}

static int lsf_open(const char *path, struct fuse_file_info *fi)
{
    if(strcmp(path, lsf_path) != 0)
        return -ENOENT;

    if((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    return 0;
}

static int lsf_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void) fi;
    if(strcmp(path, lsf_path) != 0)
        return -ENOENT;

    len = strlen(lsf_str);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, lsf_str + offset, size);
    } else
        size = 0;

    return size;
}

static int lsf_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
    size_t len;
    (void) fi;
    if(strcmp(path, lsf_path) != 0)
        return -ENOENT;
}

static struct fuse_operations lsf_oper = {
    .getattr	= lsf_getattr,
    .readdir	= lsf_readdir,
    .open	= lsf_open,
    .read	= lsf_read,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &lsf_oper);
}