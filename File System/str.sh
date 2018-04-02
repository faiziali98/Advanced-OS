fusermount -u hell
rm hell
gcc hello.c -g -lm `pkg-config fuse --cflags --libs` -o hello
mkdir hell
./hello hell -o -s -d