#define LISTENQ  1024
#define MAXLEN  1024
#define SIZE 200
int caching;

typedef struct {
  char filename[MAXLEN], filetype[MAXLEN];
  char *srcp;
  int filesize;
} filecache;

typedef struct{
   filecache *data;   
   char key[MAXLEN];
}DataItem;

void makeFile(filecache *f, int filesize, char *filename, char *filetype, char *srcp){
  f->filesize = filesize;
  strcpy(f->filename, filename);
  strcpy(f->filetype, filetype);
  f->srcp = (char*) malloc(filesize);
  memcpy(f->srcp, srcp, filesize);
}

int cacheCode(char *key) {
   return strlen(key)%SIZE;
}

DataItem* search(DataItem* cacheArray[], char *key) {
   int cacheIndex = cacheCode(key);
   int prev = cacheIndex;
   while(cacheArray[cacheIndex] != NULL) {
      if(strcmp(cacheArray[cacheIndex]->key,key)==0)
         return cacheArray[cacheIndex];
      ++cacheIndex;
      cacheIndex %= SIZE;
      if (cacheIndex == prev)
      	break;
   }        
   return NULL;        
}

void insert(DataItem* cacheArray[], char *key,filecache *data) {
   DataItem* item = (DataItem*) malloc(sizeof(DataItem));
   item->data = (filecache*) malloc(sizeof(filecache));
   makeFile(item->data, data->filesize, data->filename,
		data->filetype, data->srcp); 
   strcpy(item->key,key);
   int cacheIndex = cacheCode(key);
   int prev = cacheIndex;
   while(cacheArray[cacheIndex] != NULL) {
      ++cacheIndex;
      cacheIndex %= SIZE;
      if (cacheIndex == prev)
      	break;
   }
   cacheArray[cacheIndex] = item;
}

void display(DataItem* cacheArray[]) {
   int i = 0;
   for(i = 0; i<SIZE; i++) {
	   if(cacheArray[i] != NULL){
	        printf(" (%s)",cacheArray[i]->data->srcp);
	        printf("\n");
	   }

   }
   printf("\n");
}