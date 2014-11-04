#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define null 0
#define true 1 
#define false 0

/******************************************************************************
 ******************************************************************************
				Bounded Queue(Buffer) Producer Consumer Problem: 
    Producer waits when queue becomes full, consumer waits when queue becomes empty.
    Producer singals(wakeup) consumer when queue size becomes 1, Consumer wakes up
    Producer when queue size becomes < 0; 

 1) Producer Thread produces monotonically increasing Sequenece of integer and fills
    Queue until it gets full, once full it waits on condition "fullCond", and would be 
    woken up by consumer when queue becomes empty. 
  	Any time if size of queue becomes 1, it singals consumer to start consuming by sending
    signal on "emptCond".
 
 2) Cosumer consumes integer sequence and waits on condition "emptyCond", if size = 0; 
    Later when producer put new integer in queue, it gets woken up by producer and starts
    consuming.


Use cases to be verified:
    1) Producer starts early, fills in buffer full, and later consumer kicks off.
       Done: P would be waiting on "fullCond", C finds full buffer, pops all items(task)
             and when  buffer size <= 0, fires condition "fullCond".
   2)  Consumer starts early, then procuder.
        Done:  C would wait on "emptyCond", P kicks off, find buffer_size <= 0,
               fills buffer, and when size = 1; fire condition, "condEmpty", but
               as P dosen't unlock mutex., C would be waiting till P fills in whole buffer.
       
TODO: 
  This can be extened to multiple producers and consumers with different favouring
       variants
      1) Favouring producers(writers) over consumers(readers)
      2) Favour Readers over Writers. 

//How to run: to see the course of action.
cc producer_consumer.c
./a.out

******************************************************************************
       Queue Implementation .

******************************************************************************/

typedef struct node Node;
typedef struct queue Queue;
struct node {
  int  data;
  Node* next; 
} ;

struct queue {
  Node* head;
  Node* tail;
};


// push at end FIFO
void push(Queue* q, int data) {
   if(null == q) return;
   Node* no = malloc(sizeof(Node));
   no->data = data;
   no->next = null;
   if(q->head == null) {
        q->head = q->tail = no; 
        return;
   }
   q->tail->next = no;
   q->tail = no;
}

// pop form front;
int pop(Queue* q) {
   if (q == null || q->head == null) return -1;
   Node* tmp = q->head;
   q->head = q->head->next;
   if (q->head == null) q->tail = null;
   return tmp->data;
}

void initQueue(Queue** q) {
   *q = malloc(sizeof(Queue));
   (*q)->head = null;
   (*q)->tail = null;
} 

void printQueue(Queue* q) {
  if (null == q || q->head == null) {
	  printf(" Empty Queue\n") ;
	 return;
	}
  Node* start = q->head;

 printf(" Queue- entries ::->") ;
  while (start != null  && start <= q->tail) {
    printf(" %d ->", start->data);
	start = start->next;
  }
}

/* increasing sequnce of numbers*/
volatile int seq = 0;
int nextSequence() {
       return ++seq; 
}

/*****************************
  Thread specific functions.
*****************************
*/
#define MAX_Q_LEN  10


// locks and conditions for producer/consumer.
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t fullCond = PTHREAD_COND_INITIALIZER;

Queue* gQ;
int queueLen = 0;
volatile int flush;

void * producerFunc(void* args) {
    printf("\n\n (P)producerFunc thread  : Started\n"); 
    pthread_mutex_lock(&qlock);
    while (true) {
			while (queueLen < MAX_Q_LEN) {
				  // push next sequence in queue;
				  int seq = nextSequence();
				  push(gQ, seq); 
				  printf("(P)Producer : Pushed = %d\n", seq); 
				  queueLen++;
				  if (queueLen == 1)  {
					 pthread_cond_signal(&emptyCond);
				  }
			}
		   printf("(P)Producer :   queue = \n");
           
           printQueue(gQ);
           flush++;
		pthread_cond_wait(&fullCond, &qlock);
        sleep(5); // placing it above pthread_cond_wait will hold mutex qlock in
                  // locked state while sleeping, i.e consumer would also be
                  // waiting as qlock would not be released. 
    }
    pthread_mutex_unlock(&qlock);
}

void * consumerFunc(void* args) {
    printf("\n\n (C)consumerFunc thread  : Started\n"); 
    pthread_mutex_lock(&qlock);
    while (true) {
                  printf("\n\n");
			while (queueLen > 0) {
				  // push next sequence in queue;
				  int seq = pop(gQ);  
				  printf("(C) Comsumer: Poped = %d\n", seq); 
				  queueLen--;
				  if (queueLen <= 0)  {
					 pthread_cond_signal(&fullCond);
				  }
			}
			pthread_cond_wait(&emptyCond, &qlock);
    }
    pthread_mutex_unlock(&qlock);
}

pthread_mutex_t finish_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  finish_cond = PTHREAD_COND_INITIALIZER;
pthread_t producer;
pthread_t consumer;

int finished = false;


 int main(int argc, char* argv[]) {
    // create global queue to be shared across producer and consumer.
    initQueue(&gQ);

    // create producer and consumer thread and launch.
    pthread_create(&producer, NULL, producerFunc, NULL); 
    pthread_create(&consumer, NULL, consumerFunc, NULL); 

    printf(" Main Waiting to condition (finished) from producer and conusmer   \n");
    printf("\nn");
    pthread_mutex_lock(&finish_lock);
    while (!finished) {// TODO: Actually never gets called,     
       pthread_cond_wait(&finish_cond, &finish_lock);
       printf(" Woken up by Producer Or consumer..  Good Bye..!!! ");
   }
    return 0;
 }
