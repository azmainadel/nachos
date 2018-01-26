// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

#include "producer.h"
#include "consumer.h"

#define MAX_THREADS 15

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

// void SimpleThread(void* which)
// {
//     int num;
//
//     for (num = 0; num < 5; num++) {
// 	printf("*** thread %d looped %d times\n", (int*)which, num);
//         currentThread->Yield();
//     }
// }

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest() {
    DEBUG('t', "Entering SimpleTest");

    // Thread *t = new Thread("forked thread");

    char* producerName = new char[10];
    char* consumerName = new char[10];

    for(int i = 0; i < MAX_THREADS; i++){
        sprintf(producerName, "%d", i);
        Producer* producer = new Producer(producerName);

        Thread* thread = new Thread(producer->getName());
        thread->Fork(producer->produce, (void*) producerName);
    }

    for(int i = 0; i < MAX_THREADS; i++){
        sprintf(consumerName, "%d", i);
        Consumer* consumer = new Consumer(consumerName);

        Thread* thread = new Thread(consumer->getName());
        thread->Fork(consumer->consume, (void*) consumerName);
    }

    // t->Fork(SimpleThread, (void*) 1);
    // SimpleThread(0);
}
