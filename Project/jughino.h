#ifndef _JUGHINO_
	#define _JUGHINO_
	#include <stdio.h>
	#include <sys/types.h>
	#define STATO_UNUSED 0
	#define STATO_CREATO 1
	#define STATO_ATTESA 2
	#define STATO_CHECKING 3
	#define STATO_ACCOPPIANDO 4
	typedef struct ughino{
		int tipo;
		char nome[(50+1)];
		unsigned long genoma;
		pid_t pid;
		int semid;
		int stato;
	}jughino;
	extern void show_ugo(jughino);
	extern int getpos_bypid(jughino*, int, pid_t);
#endif
