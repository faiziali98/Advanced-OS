#include "client.h"

int main(int argc, char *argv[]){
  char *host, *filename, *torec;
  int port;
  int clientfd;

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
    exit(1);
  }

  host = argv[1];
  port = atoi(argv[2]);
  filename = argv[3];


  clientfd = open_clientfd(host, port);

  clientGet(clientfd, filename);
  clientSave(clientfd, filename);
    
  close(clientfd);

  exit(0);
}