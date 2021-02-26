#include "jpreferences.h"
/*
	Questo metodo prende in input, il path di un file contenente i valori delle settings
	con cui vorremo eseguire la nostra simulazione, ritorna il puntatore
	all'area di memoria contenente questi dati.
	Ritorna NULL in caso di errore
*/
jsettings* init_settings(char * path){
	FILE* file = fopen(path, "r");
	jsettings* settings = (jsettings*)malloc(sizeof(jsettings));
	if(file != NULL){
		char line[32];
		int cont = 0;
		while(fgets(line, sizeof(line), file) != NULL){
			switch(cont){
				case 0:
					settings->init_people = atoi(line);
					break;
				case 1:
					settings->genes = atoi(line);
					break;
				case 2:
					settings->birth_death = atoi(line);
					break;
				case 3:
					settings->sim_time = atoi(line);
					break;
			}
			cont++;
		}
		fclose(file);
	}
	return settings;
}
/*
	Questo metodo prende in input, le stats del programma ed un nuovo elemento della popolazione,
	controlla se può entrare nei record del nome massimo o del genoma massimo,
	in caso di successo, vengono copiati i suoi dati nella struttura apposita in stats.
*/
void ughino_worldrecord(jstats* stats, jughino ugo){
	if((stats -> ugo_genoma_max).genoma < ugo.genoma){
		(stats -> ugo_genoma_max).tipo = ugo.tipo;
		strcpy((stats -> ugo_genoma_max).nome,ugo.nome);
		(stats -> ugo_genoma_max).genoma = ugo.genoma;
		(stats -> ugo_genoma_max).pid = ugo.pid;
		(stats -> ugo_genoma_max).stato = ugo.stato;
	}
	if((strlen((stats -> ugo_nome_max).nome) < strlen(ugo.nome)) || ((strlen((stats -> ugo_nome_max).nome) == strlen(ugo.nome)) && (strcmp((stats -> ugo_nome_max).nome, ugo.nome) < 0))){
		(stats -> ugo_nome_max).tipo = ugo.tipo;
		strcpy((stats -> ugo_nome_max).nome,ugo.nome);
		(stats -> ugo_nome_max).genoma = ugo.genoma;
		(stats -> ugo_nome_max).pid = ugo.pid;
		(stats -> ugo_nome_max).stato = ugo.stato;
	}
}
/*
	Questo metodo crea un puntatore a delle stats, inizializzate a 0 e pronte all'uso
	Ritorna il puntatore all'area di memoria contenente questi dati.
*/
jstats* init_stats(){
	jstats* stats = (jstats*)malloc(sizeof(jstats));
	(stats -> n_processi[0]) = 0;
	(stats -> n_processi[1]) = 0;
	(stats -> n_processi_max[0]) = 0;
	(stats -> n_processi_max[1]) = 0;
	(stats -> ugo_nome_max).tipo = 0;
	(stats -> ugo_nome_max).nome[0] = '\0';
	(stats -> ugo_nome_max).genoma = 0;
	(stats -> ugo_nome_max).pid = 0;
	(stats -> ugo_nome_max).stato = 0;
	(stats -> ugo_genoma_max).tipo = 0;
	(stats -> ugo_genoma_max).nome[0] = '\0';
	(stats -> ugo_genoma_max).genoma = 0;
	(stats -> ugo_genoma_max).pid = 0;
	(stats -> ugo_genoma_max).stato = 0;
	return stats;
}
/*
	Questo metodo mostra i valori delle settings contenuti nel jsettings passato in ingresso
*/
void show_settings(jsettings* s){
	printf("\n_____________________________________________________________________\n\n");
	printf("		-> VALORI SETTINGS <-\n");
	printf("\n	init_people: %d", s->init_people);
	printf("\n	genes: %d", s->genes);
	printf("\n	birth_death: %d", s->birth_death);
	printf("\n	sim_time: %d", s->sim_time);
	printf("\n_____________________________________________________________________\n");
}
/*
	Questo metodo mostra i valori delle stats contenute nel jstats passato in ingresso
*/
void show_stats(jstats* s){
	printf("\n_____________________________________________________________________\n\n");
	printf("		-> VALORI STATS <-\n");
	printf("\n	Numero processi A a fine simulazione: %d", s->n_processi[0]);
	printf("\n	Numero processi B a fine simulazione: %d", s->n_processi[1]);
	printf("\n	Numero processi A vissuti: %d", s->n_processi_max[0]);
	printf("\n	Numero processi B vissuti: %d", s->n_processi_max[1]);
	printf("\n--------------------------------------------------------");
	printf("\n	Processo con nome massimo:\n");
	show_ugo(s->ugo_nome_max);
	printf("\n	Processo con genoma massimo:\n");
	show_ugo(s->ugo_genoma_max);
	printf("\n_____________________________________________________________________\n");
}
/*
	Metodo di creazione dei dati di un nuovo Processo di tipo BASE(i primi processi creati/processi creati durante il birth_death)
*/
jughino crea_ughino_primordiale(jstats* stats, int genoma, pid_t pid){
	jughino ugo;
	ugo.tipo = rand() % 2; //tipo 0/A OPPURE 1/B
	ugo.nome[0] = (rand() % 26) + 65; //lettere maiuscole sono comprese in 65 <-> 95
	ugo.nome[1] = '\0';
	ugo.genoma = ((rand() % (genoma + 1)) + 2) ; //gene random fra 2 <-> 2 + genes
	ugo.pid = pid; //pid del processo
	ugo.stato = STATO_CREATO; //stato appena creato
	if(stats->n_processi[0] == 0 || (stats->n_processi[1] > 0 && ugo.tipo == 0))
		ugo.tipo = 0;
	else if (stats->n_processi[1] == 0 || (stats->n_processi[0] > 0 && ugo.tipo == 1))
		ugo.tipo = 1;
	stats->n_processi[ugo.tipo]++;
	stats->n_processi_max[ugo.tipo]++;
	ughino_worldrecord(stats,ugo);
	return ugo;
}
/*
	Metodo di creazione dei dati di un nuovo processo di tipo FIGLIO(i processi nati da accoppiamento)
*/
jughino crea_ughino_non_primordiale(jstats* stats, int genoma, pid_t pid, int mcdgenes, char* nome){
	jughino ugo;
	ugo.tipo = rand() % 2; //tipo 0/A OPPURE 1/B
	sprintf(ugo.nome, "%s", nome);//strcpy(ugo.nome, nome);
	char c[] = {'A', '\0'};
	c[0] = ((rand() % 26) + 65);
	c[1] = '\0';
	strcat(ugo.nome, c);
	ugo.genoma = ((rand() % (genoma + 1)) + mcdgenes) ; //gene random fra mcdgenes <-> mcdgenes + genes
	ugo.pid = pid; //pid del processo
	ugo.stato = STATO_CREATO; //stato appena creato
	if(stats->n_processi[0] == 0 || (stats->n_processi[1] > 0 && ugo.tipo == 0))
		ugo.tipo = 0;
	else if (stats->n_processi[1] == 0 || (stats->n_processi[0] > 0 && ugo.tipo == 1))
		ugo.tipo = 1;
	stats->n_processi[ugo.tipo]++;
	stats->n_processi_max[ugo.tipo]++;
	ughino_worldrecord(stats, ugo);
	return ugo;
}
/*
	Metodo per calcolare il MASSIMO COMUN DIVISORE fra due numeri interi
*/
int MCD(int a, int b){
	int x;
	if (a < b){
		x = a;
		a = b;
		b = x;
	}
	x = b;
	while((a % x != 0) || (b % x != 0))
		x--;
	return x;
}
