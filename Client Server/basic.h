#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include "cache.h"

int c_client = 0;
DataItem *cacheArray[SIZE];
pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
typedef struct sockaddr SA;

ssize_t writes(int fd, void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR){
                nwritten = 0;
            }else{
                return -1;
              }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

ssize_t readline(int fd, void *usrbuf, size_t maxlen, void *rest) {
    int n, rc;
    char buf[2048];
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
          *bufp++ = c;
          if (c == '\n')
            break;
        } else
          return -1;
    }
    if (rest!=NULL){
      if (read(fd,buf,2048)<0) return -1;
      memcpy(rest, buf, 2048);
    }
    *bufp = 0;
    return n;
}

void reportError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
   char buf[MAXLEN], body[2048];

   if (!c_client){
     sprintf(body, "<html><title>Advanced OS Error</title>");
     sprintf(body, "%s<body bgcolor=""fffff"">\r\n", body);
     sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
     sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
     sprintf(body, "%s<hr>Faizan's Web Server\r\n", body);

     sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
     if (writes(fd, buf, strlen(buf))<0) return;
     sprintf(buf, "Content-Type: text/html\r\n");
     if (writes(fd, buf, strlen(buf))<0) return;
     sprintf(buf, "Content-Length: %lu\r\n\r\n", strlen(body));
     if (writes(fd, buf, strlen(buf))<0) return;

     if (writes(fd, body, strlen(body))<0) return;
   }else{
    sprintf(body, "Error");
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sContent-Length: %d\r\n", buf, (int)strlen(body));
    sprintf(buf, "%sContent-Type: text/html\r\n\r\n", buf);
    if (writes(fd, buf, strlen(buf))<0) return;
    if (writes(fd, body, strlen(body))<0) return;
   }
}

int open_server(int port) {
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(stderr, "socket failed\n");
      return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0) {
      fprintf(stderr, "setsockopt failed\n");
      return -1;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
      fprintf(stderr, "bind failed\n");
      return -1;
    }

    if (listen(listenfd, LISTENQ) < 0) {
      fprintf(stderr, "listen failed\n");
      return -1;
    }
    return listenfd;
}

void getFileName(char *uri, char *filename) {
   sprintf(filename, ".%s", uri);
   if (uri[strlen(uri)-1] == '/') {
      strcat(filename, "index.html");
   }
}

void getFileType(char *filename, char *filetype){
   if (strstr(filename, ".html"))
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif"))
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg"))
      strcpy(filetype, "image/jpeg");
   else if (strstr(filename, ".jpeg"))
      strcpy(filetype, "image/jpeg");
   else
      strcpy(filetype, "test/plain");
}

void getFileTypes(char *filename, char *filetype){
   if (strstr(filename, ".html"))
      strcpy(filetype, "html");
   else if (strstr(filename, ".jpg"))
      strcpy(filetype, "jpg");
   else if (strstr(filename, ".jpeg"))
      strcpy(filetype, "jpeg");
   else
      strcpy(filetype, "test/plain");
}

int sendHeader(int fd_client, int filesize, char *filetype, char *srcp){
  char buf[2048];
  sprintf(buf, "HTTP/1.1 200 OK\r\n");
  sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);
  if (writes(fd_client, buf, strlen(buf))<0) return -1;
  if (writes(fd_client, srcp, filesize)<0) return -1;
  return 1;
}

int sendCachedFile(int fd_client, filecache* item){
  printf("sending cached file\n");
  if (sendHeader(fd_client, item->filesize, item->filetype, item->srcp)<0) return -1;
}

int getFile(int fd_client, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLEN];
  getFileType(filename, filetype);
  srcfd = open(filename, O_RDONLY, 0);
  srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  if (sendHeader(fd_client, filesize, filetype, srcp)<0) return -1;

  if (caching){
    filecache *f = (filecache*) malloc(sizeof(filecache));
    makeFile(f, filesize, filename, filetype, srcp);
    pthread_mutex_lock(&cache_lock);
    insert(cacheArray, filename, f);
    pthread_mutex_unlock(&cache_lock);
  }

  munmap(srcp, filesize);
  return 1;
}

int checks(int fd_client, char *filename){
  if (caching){
    pthread_mutex_lock(&cache_lock);
    DataItem* item = search(cacheArray, filename);
    pthread_mutex_unlock(&cache_lock);

    if (item!=NULL){
      if (sendCachedFile(fd_client, item->data)<0) return -1;
      return 1;
    }
  }
  struct stat sbuf;
  if (stat(filename, &sbuf) < 0) {
    printf("Error 1\n");
    reportError(fd_client, filename, "404", "Not found", "Server could not find this file");
    return -1;
  }
  if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
    printf("Error 2\n");
    reportError(fd_client, filename, "403", "Forbidden", "Server could not read this file");
    return -1;
  }
  if (getFile(fd_client, filename, sbuf.st_size)<0) return -1;
  return 1;
}
