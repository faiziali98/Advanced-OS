#include "basic.h"


void clientGet(int fd, char *filename){
  char buf[MAXLEN];
  char hostname[MAXLEN];

  gethostname(hostname, MAXLEN);

  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  writes(fd, buf, strlen(buf));
}

int open_clientfd(char *hostname, int port) {
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if ((hp = gethostbyname(hostname)) == NULL)
        return -2;

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, 
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

void clientSave(int fd, char *filename){
  char buf[2048];
  char filetype[strlen(filename)]; 
  int length = 0;
  int n;

  n = readline(fd, buf, 2048,NULL);

  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = readline(fd, &buf, 2048,NULL);
    printf("%d\n", n);

    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  getFileTypes(filename, filetype);
  sprintf(filename, "rec.%s", filetype);
  FILE *fp = (FILE*) malloc(sizeof(FILE));
  fp = fopen(filename ,"a");

  int tot = 0;
  n = readline(fd, buf, 2048,NULL);
  while (n > 0) {
    tot += n;
    n = readline(fd, buf, 2048,NULL);
  }
  tot += n;
  printf("%d\n", tot);
}


void *consumer(void *arg){
  clientGet(clientfd, torec);
  clientSave(clientfd, filename);
}

int main(int argc, char *argv[]){
  char *host, *filename, *torec;
  int port;
  int clientfd;
  torec = (char*) malloc(sizeof(char));

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];
  int threads = atoi(argv[4]);

  cid = malloc(threads*sizeof(*cid));

    int i;
    for(i = 0; i< threads; i++) {
        cid[i] = malloc(sizeof(pthread_t));
        pthread_create(cid[i], NULL, consumer, NULL);
    }

  printf("%s\n",filename);
  sprintf(torec, "/%s",filename);

  clientfd = open_clientfd(host, port);
    
  close(clientfd);

  exit(0);
}