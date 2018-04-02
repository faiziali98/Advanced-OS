
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

static int files = 1000;
static int files_start = 40;

static char blocks[1000][1000];



static int get_inode_num(int num)
{
	
}

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

static int hello_getattr(const char *path, struct stat *stbuf)
{

	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	int block_num = get_block_num(path);

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (get_block_num(path) >= 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(blocks[block_num]);
	} else
		res = -ENOENT;

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	// filler(buf, hello_path + 1, NULL, 0);
	int i;
	char str[4];
	for (i = files_start; i < files; ++i){
		sprintf(str, "%d", i);
		filler(buf, str , NULL, 0);	
	}
	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (get_block_num(path) == -1)
		return -ENOENT;

	// if ((fi->flags & 3) != O_RDONLY)
	// 	return -EACCES;

	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(get_block_num(path) == -1)
		return -ENOENT;

	int block_num = get_block_num(path);
	len = strlen(blocks[block_num]);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, blocks[block_num] + offset, size+1);
	} else
		size = 0;

	return size;
}

static int hello_truncate(const char* path, off_t size){
	printf("Truncate was called. Not implemented.\n");
	return 0;
}

static int hello_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\n\n\n");
	size_t len;
	(void) fi;
	if(get_block_num(path) == -1)
		return -ENOENT;

	int block_num = get_block_num(path);

	memcpy(blocks[block_num], buf, 1024);
	// len = strlen(buf);
	// memcpy(hello_str,buf,size);

	return size;
}

static int hello_chmod(const char* path, mode_t mode){
	printf("Chmod was called.\n");
}

static int  hello_chown(const char* path, uid_t uid, gid_t gid){
	printf("Chown was called.\n");
}

static int hello_utimens(const char* path, const struct timespec ts[2]){
	printf("Utimens was called.\n");		
}
static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
	.write 		= hello_write,
	.truncate   = hello_truncate,
	.chmod 		= hello_chmod,
	.chown		= hello_chown,
	.utimens 	= hello_utimens,

};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &hello_oper, NULL);
}
