
#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

static int files = 160;
static int files_start = 40;

static char blocks[1000][1000];




static int get_inode_block(int num)
{
	num = num*40;
	int i = 0;
	while(num/1000 >= 1){
		num = num - 1000;
		i++;
	}
	return i;

}

static int get_inode_location(int num)
{
	num = num*40;
	int i = 0;
	while(num/1000 >= 1){
		num = num - 1000;
		i++;
	}
	return num;
}

static int get_block_num(const char* path)
{
	char * filename = malloc(strlen(path));
	memcpy(filename, path+1, strlen(path));
	// printf("%s this shit caused problem\n", filename);
	int i = 0;
	char temp[4];
	for (i=files_start; i<files;i++){
		sprintf(temp, "%d", i-40);
		if (strcmp(filename,temp) == 0){
			printf("file %d exist\n",i-40);
			return (i-40);
		}
	}
	return -1;
}

static void set_inode_sizes(int num, int content)
{
	char temp[4];
	sprintf(temp,"%d",content);
	int temp_block = get_inode_block(num);
	int temp_location = get_inode_location(num);
	memcpy(blocks[temp_block]+temp_location,temp,4);
}

static void set_inode(int num)
{
	char temp[4];
	int first = num*8+40;
	int temp_block = get_inode_block(num);
	int temp_location = get_inode_location(num);
	int i = 0;
	for (i = 0;i<8;i++){
		sprintf(temp,"%d",num*8+40+i);
		memcpy(blocks[temp_block]+temp_location+4+i*4,temp,4);
	}
}


static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	int inode_num = get_block_num(path);
	int data_block_row = get_inode_block(inode_num);
	int data_block_col = get_inode_location(inode_num);
	char temp[4];
	int i;
	memcpy(temp,blocks[data_block_row]+data_block_col,4);
	sscanf(temp, "%d", &i);
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (get_block_num(path) >= 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = i;
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
		sprintf(str, "%d", i-40);
		// printf("%d sent from read dir\n", i-40);
		set_inode_sizes(i-40,0);
		set_inode(i-40);
		filler(buf, str , NULL, 0);	
	}
	return 0;
}



static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (get_block_num(path) == -1)
		return -ENOENT;
	int inode_num = get_block_num(path);
	int data_block_row = get_inode_block(inode_num);
	int data_block_col = get_inode_location(inode_num);
	char temp[4];
	int size;
	memcpy(temp,blocks[data_block_row]+data_block_col,4);
	sscanf(temp, "%d", &size);
	// printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n %d \n %d\n %d \n %d\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",inode_num,data_block_row,data_block_col,size);
	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{


	size_t len;
	(void) fi;
	int err_check = get_block_num(path);
	if(err_check == -1){
		printf("%d this is where it ends\n",err_check);
		return -ENOENT;
	}
	int inode_num = get_block_num(path);
	int data_block_row = get_inode_block(inode_num);
	int data_block_col = get_inode_location(inode_num);
	char temp[4];
	int size1= 0;
	int i;
	memcpy(temp,blocks[data_block_row]+data_block_col,4);
	sscanf(temp, "%d", &size1);
	memcpy(temp,blocks[data_block_row]+data_block_col+4,4);
	sscanf(temp, "%d", &i);
	// char* content_temp = malloc(size+1);


	len = size1;
	size = size1;
	if (offset < len) {
		int j=0;
		int bytes_read = 0;
		while(bytes_read < len) {
	// printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nF this shit read\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");	
			if (1000 > len - bytes_read){
				size = len - bytes_read;
				memcpy(buf+j*1000,blocks[i+j],size+1);
				bytes_read = bytes_read + size;
				break;
				// memcpy(buf, blocks[i+j] + offset, size+1);
			}else{
				bytes_read = bytes_read + 1000;
				memcpy(buf+j*1000,blocks[i+j],1000);
				// memcpy(buf, blocks[i+j], 1000);
			}
			printf("%d %d %d this is the result you are looking for\n", j,bytes_read,size1);
			j++;
		}
	} else
		size = 0;

	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nF this shit read\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");	
	// printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nF this shit\n%s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",buf);	

	return size1;
}

static int hello_truncate(const char* path, off_t size){
	printf("Truncate was called. Not implemented.\n");
	return 0;
}

static void our_memcopy(char* temp, char* blocks1, int len)
{
	temp[0] = blocks1[0];
	temp[1] = blocks1[1];
	temp[2] = blocks1[2];
	temp[3] = blocks1[3];

}
static int hello_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	size_t len;
	(void) fi;
	if(get_block_num(path) == -1)
		return -ENOENT;
	int inode_num = get_block_num(path);
	int data_block_row = get_inode_block(inode_num);
	int data_block_col = get_inode_location(inode_num);
	char temp[4];
	int i;
	int size1 = size;
	sprintf(temp, "%d", size1);
	memcpy(blocks[data_block_row]+data_block_col,temp,4);

	memcpy(temp,blocks[data_block_row]+data_block_col+4,4);
	sscanf(temp, "%d", &i);

	int j = 0;
	while( size1/1000 >=1){
		memcpy(blocks[i+j], buf+j*1000, 1024);
		size1 = size1 -1000;
		j++;
	}
	memcpy(blocks[i+j], buf+j*1000, size1+1);
	char* content_temp = malloc(strlen(buf));
	memcpy(content_temp,blocks[data_block_row]+data_block_col,4);
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
