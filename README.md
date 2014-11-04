ProducerConsumerBounderBuffer
=============================

ProducerConsumerBounderBuffer, C implementation of Producer Consumer with Bounded queue.
 This for Fun to understand pthread based mutext and condition based implemenation.

Feel Free add more complex use-cases.

/******************************************************************************
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
