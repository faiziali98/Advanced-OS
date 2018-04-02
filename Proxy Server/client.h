#include "basic.h"

int adr;
char * HOSTNAME = "localhost";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t **cid;

void clientGetLocal(int fd, char *filename, int tid){
	char buf[MAXLEN];
	char hostname[MAXLEN];
	gethostname(hostname, MAXLEN);

	sprintf(buf, "GET_LOCAL %s HTTP/1.0 %d\n", filename, tid);
	sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
	writes(fd, buf, strlen(buf),0);
}

void clientGet(int fd, char *filename){
	char buf[MAXLEN];
	char hostname[MAXLEN];

	gethostname(hostname, MAXLEN);

	sprintf(buf, "GET %s HTTP/1.0\n", filename);
	sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
	writes(fd, buf, strlen(buf),0);
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
  int no_err = 1;

  n = readline(fd, buf, 2048,NULL);

  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    if (strstr(buf, "404") != NULL){
    	no_err = 0;
    }
    n = readline(fd, &buf, 2048,NULL);
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
    	printf("Length = %d\n", length);
    }
  }

  if (no_err){
	  getFileTypes(filename, filetype);
	  sprintf(filename, "rec.%s", filetype);
	  FILE *fp = (FILE*) malloc(sizeof(FILE));
	  fp = fopen(filename ,"a");

	  n = readline(fd, buf, 2048,NULL);
	  while (n > 0) {
	    n = readline(fd, buf, 2048,NULL);
	  }
   }
}

void SendBack(int fd, int bd){
	char buf[2048];
	int n;

	n = readline(fd, buf, 2048,NULL);
	while (n > 0) {
	  	writes(bd,buf,n,0);
		n = readline(fd, buf, 2048,NULL);
	}
}


void SendBackLocal(int ser, int fd, int tid){
	shm_data *dt = shm+(tid*sizeof(shm_data));
	char buf[2048];
	readline(ser, buf, 2048,NULL);

	if (strstr(buf,"OK")){
		while (1){
			pthread_mutex_lock(&(dt->shm_l));

			while (!(dt->filled)){
				pthread_cond_wait(&(dt->shm_cr), &(dt->shm_l));
			}

			if (strstr(dt->data,"!end!")){
				dt->filled = 0;
		  		pthread_mutex_unlock(&(dt->shm_l));
		  		pthread_cond_signal(&(dt->shm_cw));
	  			break;
			}

			writes(fd,dt->data,sizeof(dt->data),0);

	  		dt->filled = 0;
	  		pthread_mutex_unlock(&(dt->shm_l));
	  		pthread_cond_signal(&(dt->shm_cw));
		}
	}else{
		writes(fd,buf,sizeof(buf),0);
		SendBack(ser,fd);
	}
}