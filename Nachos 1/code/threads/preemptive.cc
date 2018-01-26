// preemptive.cc 
//	Extension to make kernel threads be periodically preempted
//      It only works on Linux x86 environments
//
// Copyright (c) 2007 Universidad de Las Palmas de Gran Canaria
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for any purpose, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice appear in all 
// copies of this software.

#include "preemptive.h"

// access to global object: currentThread, interrupt...
#include "system.h"

// UNIX and Linux-specific headers
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

static void ContextSwitch ();
static void MonitorProcess ( int childPid, unsigned long timeSliceLength );
static void LetMeBeMonitored ();

static bool inContextSwitch = false;

// Set up the preemptive scheduler
// The 'timeSliceLength' argument means how many machine instructions
// will last the time slice for every kernel thread

void PreemptiveScheduler::SetUp ( unsigned long timeSliceLength )
{
  int  childPid = fork();
  switch ( childPid ) {
    case -1:
      DEBUG ( 'p', "Preemptive scheduler: unable to launch child process\n" );
      ASSERT (false);
      break;
    
    // child process: original Nachos code
    case 0:
      LetMeBeMonitored ();
      // resumes Nachos execution as a child process
      return;
    
    // parent process: monitor code
    default:
      MonitorProcess ( childPid, timeSliceLength );
    } // end of switch
}


void LetMeBeMonitored ()
{
  // allow the parent process to ptrace me
  ptrace ( PTRACE_TRACEME, 0, NULL, NULL );
  
  DEBUG ( 'p', "Preemptive scheduler: child process will be ptraced\n" );
  
  // Raise a signal to bring back control to the parent
  raise(SIGUSR1);
}


void MonitorProcess ( int childPid, unsigned long timeSliceLength )
{
  // machine instruction counter
  long long instructionCounter = 1;
  
  while (true) {
    
    // Wait for child process
    int wait_val;
    wait (&wait_val);
    if ( WIFEXITED(wait_val) ) {
      DEBUG ( 'p', "Preemptive scheduler: child process terminated\n" );
      break;
    }

    // Increment instruction counter
    instructionCounter++;
    
    // From time to time, insert machine code to force a context switch
    if ( instructionCounter % timeSliceLength == 0 )
    {
      // Get child value of 'inContextSwitch'
      long incs = ptrace ( PTRACE_PEEKDATA, childPid, (long)&inContextSwitch, NULL );
      
      if ( incs == 0 ) { 
        DEBUG ( 'p', "Preemptive scheduler: "
                     "forcing a context switch at instruction %lld\n",
                     instructionCounter );
                     
        struct user_regs_struct regs;
        
        // Get child process registers
        ptrace ( PTRACE_GETREGS,  childPid, NULL, &regs );

	// build a call stack frame by inserting EIP/RIP on top of the stack
#ifdef HOST_i386        
        regs.esp = regs.esp - 4;
        ptrace ( PTRACE_POKEDATA, childPid, regs.esp, regs.eip );
        // force a jump to ContextSwitch()
        regs.eip = (long) ContextSwitch;
#elif defined(HOST_x86_64)
        regs.rsp = regs.rsp - 8;
        ptrace ( PTRACE_POKEDATA, childPid, regs.rsp, regs.rip );
        // force a jump to ContextSwitch()
        regs.rip = (long long) ContextSwitch;
#endif

        // Store new register values in the child process
        // The child will now make a call to ContextSwitch()
        ptrace ( PTRACE_SETREGS, childPid, NULL, &regs );
      }
    }
    
    // Resume child execution
    ptrace( PTRACE_SINGLESTEP, childPid, NULL, NULL );
  }

  DEBUG ( 'p', "Preemptive scheduler: Finished\n" );
  exit(0);
}


// Force a context switch
// This call is made asynchronously from the
// parent process, using ptrace() features

static void ContextSwitch (void)
{
  // prevent register loss: save all registers
#ifdef HOST_i386        
  __asm__ ("pushal");
  __asm__ ("subl $8, %esp");
#elif defined(HOST_x86_64)
  // push all registers
  __asm__ ("push %rax; push %rbx; push %rcx; push %rdx");
  __asm__ ("push %rsi; push %rdi; push %rsp; push %rbp");
  __asm__ ("push %r8; push %r9; push %r10; push %r11; push %r12; push %r13; push %r14; push %r15");
  __asm__ ("sub $16, %rsp");
#endif
  
  inContextSwitch = true;
  
  // make a context switch if interrupts are enabled
  if ( interrupt->getLevel() == IntOn ) {
    inContextSwitch = false;
    currentThread->Yield();
  } else {
    interrupt->YieldOnReturn();
    inContextSwitch = false;
  }
  
  // restore old register values
#ifdef HOST_i386        
  __asm__ ("addl $8, %esp");
  __asm__ ("popal");
#elif defined(HOST_x86_64)
  __asm__ ("add $16, %rsp");
  // pop all registers
  __asm__ ("pop %r15; pop %r14; pop %r13; pop %r12; pop %r11; pop %r10; pop %r9; pop %r8");
  __asm__ ("pop %rbp; pop %rsp; pop %rdi; pop %rsi");
  __asm__ ( "pop %rdx; pop %rcx; pop %rbx; pop %rax" );
#endif

}

