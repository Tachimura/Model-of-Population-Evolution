#ifndef _JCOMUNICATOR_H_
	#define _JCOMUNICATOR_H_
	//!INCLUDE
	#include <stdlib.h> //malloc e free
	#include <string.h> //uso delle stringhe
	#include <sys/msg.h>
	#include <sys/shm.h>
	#include <sys/sem.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#include <signal.h>
	#include <errno.h>
	#include "jughino.h"
	//!DEFINE
	#define JMESSAGE_DIMENSION 51 //definisco una determinata dimensione per il campo messaggio
	#define ANSWER_OK 0
	#define ANSWER_NOPE 1
	#define REQUEST_ACCOPPIAMENTO 2
	
	typedef struct jmsg {
		long mtype; /* Message type */
		char mtext[JMESSAGE_DIMENSION]; /* Message body */
	}jmessage;
	  
	#if defined(__linux__)
		union semun {
			// value for SETVAL 
			int val;     
			// buffer for IPC_STAT, IPC_SET 
			struct semid_ds* buf;      
			// array for GETALL, SETALL  
			unsigned short* array;  
			// Linux specific part  
			#if defined(__linux__)
			// buffer for IPC_INFO 
			struct seminfo* __buf; 
			#endif 
		};
	#endif
	extern int send_message(int, jmessage*);
	extern int get_message(int, jmessage*, int);
	extern int clean_message(int, jmessage*, int);
	extern int open_ipc_channel();
	extern int close_ipc_channel(int);
	extern int create_semaphore();
	extern int check_and_lock_semaphore(int semid);
	extern int update_semaphore(int, int);
	extern int get_semaphore_val(int);
	extern int set_semaphore(int, int);
	extern int create_shared_memory(int);
	extern int detach_shared_memory(const void* addr);
	extern void destroy_semaphore(int);
	extern void close_shared_memory(int);
	extern void jexit(char *);
#endif
