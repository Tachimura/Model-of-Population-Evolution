#include "jughino.h"

/*
	Questo metodo mi ritorna in forma grafica, i dati del membro della popolazione passato in ingresso
*/
void show_ugo(jughino ugo){
	if(ugo.tipo == 0)
		printf("\n  ▄▀▀▀▄    ▄▀█▀▀█▄		Ugo -> tipo: A");
	else
		printf("\n  ▄▀▀▀▄    ▄▀█▀▀█▄		Ugo -> tipo: B");
	printf("\n ▄▀ ▀ ▀▄▄▀█▄▀ ▄▀ ▄▀▄		Ugo -> nome: %s", ugo.nome);
	printf("\n █▄▀█   █ █▄▄▀ ▄▀ ▄▀▄		Ugo -> genoma: %lu", ugo.genoma);
	printf("\n   █▄▄▀▀█▄ ▀▀▀▀▀▀▀ ▄█▄		Ugo -> pid: %d", ugo.pid);
	printf("\n      ▄████▀▀▀▀████ ▀█▄");
	printf("\n--------------------------------------------------------");
}
/*
	Metodo che ritorna la posizione nell'array pop il cui elemento ha il pid = al pid passato in ingresso
*/
int getpos_bypid(jughino* pop, int max, pid_t ppid){
	int pos = 0;
	for(pos = 0; pos < max; pos++){
		if(pop[pos].pid == ppid)
			break;
	}
	return pos;
}
