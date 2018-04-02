int *buffer;
int filled, curptr, tofill, bufsize;


int enqueue(int data){
	if (filled == bufsize)
		return -1;
	buffer[tofill] = data;  
    tofill = (tofill+1) % bufsize;    
    filled++;
    return 1;
}

int dequeue(int *fd){
	if (filled==0)
		return -1;
	filled--;
    *fd = buffer[curptr];
    curptr = (curptr + 1) % bufsize;
}