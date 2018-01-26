// synch.h
//	NOTA: �ste es el �nico fichero fuente con los comentarios en espa�ol
//	2000 - Jos� Miguel Santos Espino - ULPGC
//
//	Estructuras de datos para sincronizar hilos (threads)
//
//	Aqu� se definen tres mecanismos de sincronizaci�n: sem�foros
//	(semaphores), cerrojos (locks) y variables condici�n (condition var-
//	iables). S�lo est�n implementados los sem�foros; de los cerrojos y
//	variables condici�n s�lo se proporciona la interfaz. Precisamente el
//	primer trabajo incluye realizar esta implementaci�n.
//
//	Todos los objetos de sincronizaci�n tienen un par�metro "name" en
//	el constructor; su �nica finalidad es facilitar la depuraci�n del
//	programa.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// synch.h -- synchronization primitives.  

#ifndef SYNCH_H
#define SYNCH_H

#include "copyright.h"
#include "thread.h"
#include "list.h"

// La siguiente clase define un "sem�foro" cuyo valor es un entero positivo.
// El sem�foro ofrece s�lo dos operaciones, P() y V():
//
//	P() -- espera a que value>0, luego decrementa value
//
//	V() -- incrementa value, despiera a un hilo en espera si lo hay
//
// Observen que esta interfaz NO permite leer directamente el valor del
// sem�foro -- aunque hubieras podido leer el valor, no te sirve de nada,
// porque mientras tanto otro hilo puede haber modificado el sem�foro,
// si t� has perdido la CPU durante un tiempo.

class Semaphore {
  public:
    // Constructor: da un valor inicial al sem�foro  
    Semaphore(const char* debugName, int initialValue);	// set initial value
    ~Semaphore();   					// destructor
    const char* getName() { return name;}			// para depuraci�n

    // Las �nicas operaciones p�blicas sobre el sem�foro
    // ambas deben ser *at�micas*
    void P();
    void V();
    
  private:
    const char* name;        		// para depuraci�n
    int value;         		// valor del sem�foro, siempre es >= 0
    List<Thread*> *queue;       // Cola con los hilos que esperan en P() porque el
                       		// valor es cero
};

// La siguiente clase define un "cerrojo" (Lock). Un cerrojo puede tener
// dos estados: libre y ocupado. S�lo se permiten dos operaciones sobre
// un cerrojo:
//
//	Acquire -- espera a que el cerrojo est� libre y lo marca como ocupado
//
//	Release -- marca el cerrojo como libre, despertando a alg�n otro
//                 hilo que estuviera bloqueado en un Acquire
//
// Por conveniencia, nadie excepto el hilo que tiene adquirido el cerrojo
// puede liberarlo. No hay ninguna operaci�n para leer el estado del cerrojo.


class Lock {
  public:
  // Constructor: inicia el cerrojo como libre
  Lock(const char* debugName);

  ~Lock();          // destructor
  const char* getName() { return name; }	// para depuraci�n

  // Operaciones sobre el cerrojo. Ambas deben ser *at�micas*
  void Acquire(); 
  void Release();

  // devuelve 'true' si el hilo actual es quien posee el cerrojo.
  // �til para comprobaciones en el Release() y en las variables condici�n
  bool isHeldByCurrentThread();	

  private:
    const char* name;				// para depuraci�n
    // a�adir aqu� otros campos que sean necesarios
    Thread* lockedBy;
    List<Thread*> *queue;
};

//  La siguiente clase define una "variable condici�n". Una variable condici�n
//  no tiene valor alguno. Se utiliza para encolar hilos que esperan (Wait) a
//  que otro hilo les avise (Signal). Las variables condici�n est�n vinculadas
//  a un cerrojo (Lock). 
//  Estas son las tres operaciones sobre una variable condici�n:
//
//     Wait()      -- libera el cerrojo y expulsa al hilo de la CPU.
//                    El hilo se espera hasta que alguien le hace un Signal()
//
//     Signal()    -- si hay alguien esperando en la variable, despierta a uno
//                    de los hilos. Si no hay nadie esperando, no ocurre nada.
//
//     Broadcast() -- despierta a todos los hilos que est�n esperando
//
//
//  Todas las operaciones sobre una variable condici�n deben ser realizadas
//  adquiriendo previamente el cerrojo. Esto significa que las operaciones
//  sobre variables condici�n han de ejecutarse en exclusi�n mutua.
//
//  Las variables condici�n de Nachos deber�an funcionar seg�n el estilo
//  "Mesa". Cuando un Signal() o Broadast() despierta a otro hilo,
//  �ste se coloca en la cola de preparados. El hilo despertado es responsable
//  de volver a adquirir el cerrojo. Esto lo deben implementar en el cuerpo de
//  la funci�n Wait().
//  En contraste, tambi�n existe otro estilo de variables condici�n, seg�n
//  el estilo "Hoare", seg�n el cual el hilo que hace el Signal() pierde
//  el control del cerrojo y entrega la CPU al hilo despertado, quien se
//  ejecuta de inmediato y cuando libera el cerrojo, devuelve el control
//  al hilo que efectu� el Signal().
//
//  El estilo "Mesa" es algo m�s f�cil de implementar, pero no garantiza
//  que el hilo despertado recupere de inmediato el control del cerrojo.

class Condition {
 public:
    // Constructor: se le indica cu�l es el cerrojo al que pertenece
    // la variable condici�n
    Condition(const char* debugName, Lock* conditionLock);	

    // libera el objeto
    ~Condition();	
    const char* getName() { return (name); }

    // Las tres operaciones sobre variables condici�n.
    // El hilo que invoque a cualquiera de estas operaciones debe tener
    // adquirido el cerrojo correspondiente; de lo contrario se debe
    // producir un error.
    void Wait(); 	
    void Signal();   
    void Broadcast();

  private:
    const char* name;
    // aqu� se a�aden otros campos que sean necesarios
    Lock* lock;
    List<Thread*>* waitQueue;
};




//C�digo original del Nachos para las variables condici�n - NO USAR
//Nachos original code for condition variables - DO NOT USE
/*
class Condition {
  public:
    Condition(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Condition();			// deallocate the condition
    char* getName() { return (name); }
    
    void Wait(Lock *conditionLock); 	// these are the 3 operations on 
					// condition variables; releasing the 
					// lock and going to sleep are 
					// *atomic* in Wait()
    void Signal(Lock *conditionLock);   // conditionLock must be held by
    void Broadcast(Lock *conditionLock);// the currentThread for all of 
					// these operations

  private:
    char* name;
    // plus some other stuff you'll need to define
    Lock* lock;
    List<Thread*>* waitQueue;
};
*/


#endif // SYNCH_H
