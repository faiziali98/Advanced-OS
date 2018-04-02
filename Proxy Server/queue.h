int bufsize;

typedef struct queue_nodes{
	int data;
	struct queue_nodes * next_node;
}queue_node;

typedef struct{
	queue_node *head;
	queue_node *tail;
	int num;
}queue_s;

int enqueue(queue_s * queue, int data);
int dequeue(queue_s * queue, int * data);
void delete_queue(queue_s ** queuePointer);
int is_empty(queue_s * queue);

void create_queue(queue_s *newQueue){
	newQueue = malloc(sizeof(queue_s));

	if(newQueue != NULL){	
		newQueue->head = NULL;
		newQueue->tail = NULL;
	} 
}

int enqueue(queue_s * queue, int data){
	if (queue->num == bufsize)
		return -1;

	queue_node * newNode;
	if(queue->head == NULL){
		queue->head = malloc(sizeof(queue_node));
		if(queue->head == NULL)
			return 0;
		queue->head->data = data;
		queue->head->next_node = NULL;
		queue->tail = queue->head;
	}else{
		newNode = malloc(sizeof(queue_node));
		newNode->data = data;
		newNode->next_node = NULL;

		queue->tail->next_node = newNode;

		queue->tail = newNode;
	}
	queue->num = queue->num + 1;
	return 1;
}

int dequeue(queue_s * queue, int * data){
	if(is_empty(queue))
		return -1;

	queue_node *nextHead = queue->head->next_node;

	*data = queue->head->data;

	queue_node *oldHead = queue->head;
	queue->head = nextHead;
	free(oldHead);

	if(queue->head == NULL)
		queue->tail = NULL;

	queue->num = queue->num - 1;
	return 1;
}

void delete_queue(queue_s ** queuePointer){
	if(queuePointer == NULL)
		return;
	queue_node * curNode = (*queuePointer)->head;
	queue_node * temp;

	while(curNode != NULL){
		temp = curNode->next_node;
		free(curNode);
		curNode = temp;		
	}
	free(*queuePointer);
	*queuePointer = NULL;
}

int is_empty(queue_s * queue){
	return (queue->head == NULL) ? 1 : 0;
}