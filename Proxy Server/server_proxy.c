#include "client.h"
#include "arr_queue.h"
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

void requestHandle(int fd_client, int tid){
    int clientfd;
 	char method[MAXLEN], port[MAXLEN] ,uri[MAXLEN], version[MAXLEN], buf[2048];
 	char filename[MAXLEN],hostname[MAXLEN];;

    if (readline(fd_client,buf,2048, NULL)<0) return;
    sscanf(buf, "%s %s %s", method, uri, version);

    sscanf(uri, "%*[^/]%*c%*c%[^:]%*c%[^/]%s", hostname,port,filename);

    clientfd = open_clientfd(hostname, atoi(port));
    if (clientfd<0){
        reportError(fd_client, filename, "404", "Not found", "Server not found",0);
        return;
    }
    if (strcmp(hostname,HOSTNAME)!=0){
        clientGet(clientfd, filename);
        SendBack(clientfd, fd_client);
    }else{
        clientGetLocal(clientfd,filename, tid);
        SendBackLocal(clientfd, fd_client, tid);
    }
}

void *consumer(void *arg) {
    int tid = adr;
    adr++;
    int fd; 
    
    shm_data *dt = (shm_data*) malloc(sizeof(shm_data));
    dt->filled = 0;
    dt->id = tid;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(dt->shm_l), &mutex_attr);

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&(dt->shm_cr), &cond_attr);

    pthread_condattr_t cond_attrr;
    pthread_condattr_init(&cond_attrr);
    pthread_condattr_setpshared(&cond_attrr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&(dt->shm_cw), &cond_attrr);

    memcpy(shm+(tid*sizeof(shm_data)), dt, sizeof(shm_data));

    while(1) {
        pthread_mutex_lock(&lock);

        while(dequeue(&fd)<0) {
            pthread_mutex_unlock(&lock);
            pthread_mutex_lock(&lock);
        }

        printf("Thread %d Started\n", tid);
        pthread_mutex_unlock(&lock);

        requestHandle(fd,tid);
        close(fd);
        printf("Closing...\n");

        memcpy(shm+(tid*sizeof(shm_data)), dt, sizeof(shm_data));
    }
}

int main(int argc, char *argv[]){
	if(argc != 4) {
        printf("Usage: ./execute port threads bufsize\n");
        exit(1);
    }

    srand(time(NULL));
    int port = atoi(argv[1]);
    int threads = atoi(argv[2]);
    bufsize = atoi(argv[3]);
    signal(SIGINT, INThandler);

    adr = 0;
    filled = tofill = curptr = 0;
    buffer = (int*) malloc(bufsize);

	struct sockaddr_in clientAddr;
	int clientlen = sizeof(clientAddr);
    int fd_server, fd_client;
    fd_server = open_server(port);
    if (fd_server<0) exit(1);

    printf("Server is up and running.....\n");
    cid = malloc(threads*sizeof(*cid));

    if ((shmid = shmget(key, (size_t)threads*sizeof(shm_data), 0666 | IPC_CREAT)) < 0) {
        perror("shmget");
        exit(1);
    }
    if ((shm = shmat(shmid, (void *)0, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

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
            reportError(fd_client, "GET", "503","Not Available", 
                "HTTP/1.0 503 Service Unavailable",0);
            continue;
        }

    	printf ("Got client connection......\n");
    } 
}