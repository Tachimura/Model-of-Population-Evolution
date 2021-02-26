#ifndef _JPREFERENCES_
	#define _JPREFERENCES_
	#include <time.h>
	#include "jcomunicator.h"
	typedef struct stats{
		int n_processi_max[2];//A->0,B->1
		int n_processi[2];//A->0,B->1
		jughino ugo_nome_max;
		jughino ugo_genoma_max;
	}jstats;
	
	typedef struct settings{
		int init_people;
		int genes;
		int birth_death;
		int sim_time;
	}jsettings;
	//struct timespec {
	//	time_t tv_sec;        /* seconds */
	//	long   tv_nsec;       /* nanoseconds */
	//};
	
	extern void ughino_worldrecord(jstats*, jughino);
	extern jsettings* init_settings(char*);
	extern jstats* init_stats();
	extern void show_settings(jsettings*);
	extern void show_stats(jstats*);
	extern jughino crea_ughino_primordiale(jstats*, int, pid_t);
	extern jughino crea_ughino_non_primordiale(jstats*, int, pid_t, int, char*);
	extern int MCD(int, int);
#endif
