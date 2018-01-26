// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria
//

#include "copyright.h"
#include "system.h"

#include "globalbuffer.h"
#include "producer.h"
#include "consumer.h"

#define PRODUCER_CONSUMER_SIZE 5
#define QUEUE_SIZE 10

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;
    
    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** thread %s looped %d times\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
    //interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling 
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    for ( int k=1; k<=10; k++) {
      char* threadname = new char[100];
      sprintf(threadname, "Hilo %d", k);
      Thread* newThread = new Thread (threadname);
      newThread->Fork (SimpleThread, (void*)threadname);
    }
    
    SimpleThread( (void*)"Hilo 0");
}

GlobalBuffer* buf = new GlobalBuffer("table", QUEUE_SIZE);

void
SimpleProducer(void* name)
{
    char* producerName = (char*) name;
    Producer* newProducer = new Producer(producerName, buf);
    newProducer->Run();
}

void
SimpleConsumer(void* name)
{
    char* consumerName = (char*) name;
    Consumer* newConsumer = new Consumer(consumerName, buf);
    newConsumer->Run();
}

void
ProducerConsumerTest()
{
    DEBUG('s', "Entering ProducerConsumerTest\n");

    //creating producer
    for(int i = 1; i <= PRODUCER_CONSUMER_SIZE; ++i)
    {
        char* Producer = new char[100];
        sprintf(Producer, "Producer %d", i);
        Thread* newThread = new Thread (Producer);
        newThread->Fork (SimpleProducer, (void*)Producer);
    }

    //creating consumer
    for(int i = 1; i <= PRODUCER_CONSUMER_SIZE; ++i)
    {
        char* Consumer = new char[100];
        sprintf(Consumer, "Consumer %d", i);
        Thread* newThread = new Thread (Consumer);
        newThread->Fork (SimpleConsumer, (void*)Consumer);
    }

    DEBUG('s', "Ending ProducerConsumerTest\n");
}

