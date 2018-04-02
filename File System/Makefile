fusermount -u lfs_mount_dir
rm lfs_fuse
gcc lfs_fuse.c -g -lm `pkg-config fuse --cflags --libs` -o lfs_fuse
mkdir lfs_mount_dir
./lfs_fuse lfs_mount_dir -o -s -d