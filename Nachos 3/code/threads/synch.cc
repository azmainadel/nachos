// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List<Thread*>;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
    	queue->Append(currentThread);		// so go to sleep
    	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    interrupt->SetLevel(oldLevel);		// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	   scheduler->ReadyToRun(thread);
    value++;
    interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

Lock::Lock(const char* debugName) 
{    
    DEBUG('s', "Initializing lock named %s\n", name);
    name = debugName;
    lockedBy = NULL;
    queue = new List<Thread*>;
}
Lock::~Lock() 
{
    DEBUG('s', "De-allocating lock named %s\n", name);
    delete name;
    delete lockedBy;
    delete queue;
}
void Lock::Acquire() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    DEBUG('s', "Acquiring lock of %s\n", name);

    while(lockedBy != NULL)                             //someone already has this lock
    {
        queue -> Append(currentThread);
        currentThread -> Sleep();
    }
    lockedBy = currentThread;

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt

    DEBUG('s', "Acquired lock of %s\n", name);
}
void Lock::Release() 
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    DEBUG('s', "Releasing lock of %s\n", name);
    ASSERT(isHeldByCurrentThread());                    //check if currentThread locked it
    

    /*while(queue->IsEmpty()  ==  false)                //removing all queues waiting to acquire 
    {                                                   //this lock, and adding them to 
        thread = queue->Remove();                       //scheduler's readyQueue
        scheduler->ReadyToRun(thread);
    }*/
    thread = queue->Remove();
    if (thread != NULL)                                 
       scheduler->ReadyToRun(thread);
    lockedBy = NULL;

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt

    DEBUG('s', "Released lock of %s\n", name);
}
bool Lock::isHeldByCurrentThread()
{
    return lockedBy == currentThread;
}

Condition::Condition(const char* debugName, Lock* conditionLock)//const char* debugName chilo
{
    DEBUG('s', "Constructing conditional %s with lock %s\n", debugName, conditionLock->getName());
    name = debugName;
    lock = conditionLock;
    waitQueue = new List<Thread*>;
}
Condition::~Condition() 
{ 
    DEBUG('s', "De-allocating conditional %s\n", getName());
    delete name;
    delete lock;
    delete waitQueue;
}
void Condition::Wait() 
{ 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    ASSERT(lock->isHeldByCurrentThread()); 

    waitQueue->Append(currentThread);

    lock->Release();
    DEBUG('s', "Waiting for conditional %s\n", getName());
    currentThread->Sleep();
    lock->Acquire();

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}
void Condition::Signal() 
{    
    Thread* thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    ASSERT(lock->isHeldByCurrentThread());

    if(waitQueue->IsEmpty() == false)
    {
        thread = waitQueue->Remove();// (Thread*) chilo
        scheduler->ReadyToRun(thread);
        DEBUG('s', "Sent Signal to thread %s\n", thread->getName());
    }

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}
void Condition::Broadcast() 
{
    Thread* thread; 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    ASSERT(lock->isHeldByCurrentThread());

    while(waitQueue->IsEmpty() == false)
    {
        thread = (Thread*) waitQueue->Remove();
        scheduler->ReadyToRun(thread);
        DEBUG('s', "Sent Signal to thread %s\n", thread->getName());

        //or simply Signal();
    }

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}

/*
Condition::Condition(char* debugName)           //const char* debugName chilo
{
    name = debugName;
    lock = NULL;
    waitQueue = new List<Thread*>;
}
Condition::~Condition() 
{ 
    delete name;
    delete lock;
    delete waitQueue;
}
void Condition::Wait(Lock* conditionLock) 
{ 
    //ASSERT(false); 
    ASSERT(conditionLock->isHeldByCurrentThread());
    if(waitQueue->IsEmpty())
    {                                           //this is the first time someone is
        lock = conditionLock;                   //waiting for this conditional variable
    }                                           //so save the lock for further use

    ASSERT(lock == conditionLock);              //if queue is not IsEmpty, at least someone
                                                //already is waiting for this conditional variable
                                                //with a lock, now checking if threads in
                                                //the waitQueues' set lock and this
                                                //conditionLock are same
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    waitQueue->Append(currentThread);

    lock->Release();
    currentThread->Sleep();
    lock->Acquire();

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}
void Condition::Signal(Lock* conditionLock) 
{ 
    ASSERT(conditionLock->isHeldByCurrentThread());

    ASSERT(lock == conditionLock);              //if queue is not IsEmpty, at least someone
                                                //already is waiting for this conditional variable
                                                //with a lock, now checking if threads in
                                                //the waitQueues' set lock and this
                                                //conditionLock are same
    Thread* thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    if(waitQueue->IsEmpty() == false)
    {
        thread = (Thread*) waitQueue->Remove();
        scheduler->ReadyToRun(thread);
    }

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}
void Condition::Broadcast(Lock* conditionLock) 
{ 
    ASSERT(conditionLock->isHeldByCurrentThread());

    ASSERT(lock == conditionLock);              //if queue is not IsEmpty, at least someone
                                                //already is waiting for this conditional variable
                                                //with a lock, now checking if threads in
                                                //the waitQueues' set lock and this
                                                //conditionLock are same
    Thread* thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   //disable interrupt

    while(waitQueue->IsEmpty() == false)
    {
        thread = (Thread*) waitQueue->Remove();
        scheduler->ReadyToRun(thread);

        //or simply Signal(conditionLock);
    }

    (void) interrupt->SetLevel(oldLevel);               //re enable interrupt
}
*/