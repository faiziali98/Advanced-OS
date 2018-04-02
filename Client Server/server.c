#include "basic.h"
#include "arr_queue.h"
#include <pthread.h>

int adr;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t **cid;

void requestHandle(int fd_client){
 	char method[MAXLEN], uri[MAXLEN], version[MAXLEN], buf[2048];
 	char filename[MAXLEN];

    if (readline(fd_client,buf,2048, NULL)<0) return;

    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")) {
      reportError(fd_client, method, "501", "Not Implemented", "Server does not implement this method");
      return;
   	}

   	getFileName(uri, filename);

    if (checks(fd_client, filename) < 0) return;
}

void *consumer(void *arg) {
    int tid = adr;
    adr++;
    int fd; 
    while(1) {
        pthread_mutex_lock(&lock);

        while(dequeue(&fd)<0) {
            pthread_mutex_unlock(&lock);
            pthread_mutex_lock(&lock);
        }

        printf("Thread %d Started\n", tid);
        pthread_mutex_unlock(&lock);

        requestHandle(fd);
        close(fd);
        printf("Closing...\n");
    }
}

int main(int argc, char *argv[]){
	if(argc != 5) {
        printf("Usage: ./execute port threads bufsize chaching?\n");
        exit(1);
    }

    int port = atoi(argv[1]);
    caching = atoi(argv[4]);
    int threads = atoi(argv[2]);
    bufsize = atoi(argv[3]);

    adr = 1;
    filled = tofill = curptr = 0;
    buffer = (int*) malloc(bufsize);

	struct sockaddr_in clientAddr;
	int clientlen = sizeof(clientAddr);
    int fd_server, fd_client;
    fd_server = open_server(port);
    if (fd_server<0) exit(1);

    printf("Server is up and running.....\n");
    cid = malloc(threads*sizeof(*cid));

    int i;
    for(i = 0; i< threads; i++) {
        cid[i] = malloc(sizeof(pthread_t));
        pthread_create(cid[i], NULL, consumer, NULL);
    }

    while(1){
        pthread_mutex_unlock(&lock);
        printf("Waiting for a client....\n");
        fd_client = accept(fd_server, (SA *) &clientAddr, (socklen_t *) &clientlen);
        pthread_mutex_lock(&lock);

    	if (fd_client<0){
    		fprintf(stderr, "Connection failed\n");
    		continue;
    	} else if (enqueue(fd_client)<0){
            printf("error\n");
            reportError(fd_client, "GET", "503","Not Available", "HTTP/1.0 503 Service Unavailable");
            continue;
        }

    	printf ("Got client connection......\n");
    } 
}