#include "jpreferences.h"

void figlio_exec(char*, int, int, int, int, int, int);
int get_random_processo(jughino*, int);

void jsignal_handler(int);

struct timespec tosleep, remaining;
int flag_accoppiamento;

int main(int argc, char* argv[]){
	if(argc == 3){
		//!DICHIARAZIONE VARIABILI
		register int ipc_id, ipc_id_figli; //code messaggi delle richieste varie
		register int sm_pid; //id della shared dei processi
		register int sm_wid; //id della shared dei waiter
		jsettings* settings;
		jstats* stats;
		jughino* popolazione;
		int* waiters;
		jmessage msg_r;
		int now_time;
		int bd_time;
		int cont;
		int retsleep;
		pid_t pid;
		
		//!INIZIALIZZAZIONE VARIABILI
		srand(time(NULL)); //inizializzo il seme del random
		if(strlen(argv[2]) > 124)
			jexit("Nome dell'eseguibile dei processi figli deve avere al più 124 caratteri");
		settings = init_settings(argv[1]);
		if(settings->init_people < 2)
			jexit("File di configurazione non valido.");
		stats = init_stats();
		ipc_id = open_ipc_channel(); //apre un nuovo IPC e mi torna il valore
		if(ipc_id < 0)
			jexit("Errore creazione IPC GESTORE requests");
		ipc_id_figli = open_ipc_channel(); //apre un nuovo IPC e mi torna il valore
		if(ipc_id_figli < 0)
			jexit("Errore creazione IPC PROCESSI requests");
		sm_pid = create_shared_memory((sizeof(jughino) * (settings->init_people))); //creo memoria condivisa
		if(sm_pid < 0)
			jexit("Errore creazione SHARED MEMORY");
		sm_wid = create_shared_memory((sizeof(int) * (settings->init_people))); //creo memoria condivisa
		if(sm_pid < 0)
			jexit("Errore creazione SHARED MEMORY");
		popolazione = (jughino*)shmat(sm_pid, NULL, 0);
		waiters = (int*)shmat(sm_wid, NULL, 0);
		flag_accoppiamento = 0;
		for(cont = 0; cont < settings->init_people; cont++)
			waiters[cont] = 0;
		//!TERMINE INIZIALIZZAZIONE VARIABILI
		printf("\n_____________________________________________________________________");
		show_settings(settings);
		printf("\n	IPC_ID_GESTORE -> %d\n", ipc_id);
		printf("	IPC_ID_PROCESSI -> %d\n", ipc_id_figli);
		printf("	SHARED_MEMORY -> %d\n", sm_pid);
		printf("	SHARED_MEMORY_WAITERS -> %d\n", sm_wid);
		printf("\n_____________________________________________________________________");
		printf("\n_____________________________________________________________________\n");
		
		//!INIZIO CREAZIONE UGHINI
		printf("\n		-> INIZIO GENERAZIONE UGHINI <-\n\n");
		for(cont = 0; cont < settings->init_people; cont++){
			pid = fork();
			switch(pid){
				case -1:
					printf("\nGestore: ERROR CREATING UGHINO :C\n");
					cont--;
					break;
				case 0:
					//sleep(5);
					if(get_message(ipc_id_figli, &msg_r, getpid()) < 0)//attendo il segnale di partenza dal gestore
						printf("\nUghino B %d, Error getting message from IPC: %d\n", getpid(), ipc_id_figli);
					figlio_exec(argv[2], ipc_id, ipc_id_figli, sm_pid, sm_wid, popolazione[cont].tipo, settings->init_people);
					break;
				default:
				    	//kill(pid, SIGSTOP);
					popolazione[cont] = crea_ughino_primordiale(stats, settings->genes, pid);
					popolazione[cont].semid = create_semaphore();
					printf("\nGestore: Ughino creato:");
					show_ugo(popolazione[cont]);
					break;
			}
		}
		printf("\nGestore: Premere INVIO per far partire il programma...");
		getchar(); //attendo un input da KEYBOARD per far partire il tutto
		
		//!INIZIALIZZO LE SIGNAL
		signal(SIGUSR1, jsignal_handler);
		signal(SIGUSR2, jsignal_handler);
		//RISVEGLIO I PROCESSI
		for(cont = 0; cont < settings->init_people; cont++){
			//MANDO LA PARTENZA AI FIGLI
			msg_r.mtype = popolazione[cont].pid;
			sprintf(msg_r.mtext, "GO");
			if(send_message(ipc_id_figli, &msg_r) < 0)
				printf("\nGestore: Error sending message to ughino %d\n", popolazione[cont].pid);
		}
		
		//!INIZIALIZZO I TIMER
		now_time = 0;
		bd_time = settings->birth_death;
		//!INIZIA IL LAVORO DEL GESTORE
		do{
			if(bd_time == 0){//gestisco la uccisione/creazione nuovo ughino
				//printf("\nPadre: BIRTH_DEATH_HAPPENED\n");
				bd_time = settings->birth_death;
				int elim = get_random_processo(popolazione, settings->init_people);
				int fantozzi = 0;
				if(elim >= 0){
					if(kill(popolazione[elim].pid, SIGTERM) < 0)
						printf("\nGestore: ERROR KILLING UGHINO %d\n", popolazione[elim].pid);
					else{
						printf("\nGestore: BIRTH_DEATH Ughino ucciso:");
						show_ugo(popolazione[elim]);
						stats->n_processi[popolazione[elim].tipo]--;
						popolazione[elim].stato = STATO_UNUSED;
						popolazione[elim].tipo = 0;
						popolazione[elim].nome[0] = '\0';
						popolazione[elim].genoma = 0;
						popolazione[elim].pid = 0;
						update_semaphore(popolazione[elim].semid, 1);
						//!CREO NUOVO UGHINO
						pid = fork();
						switch(pid){
							case -1:
								printf("\nGestore: ERROR CREATING UGHINO :C\n");
								break;
							case 0:
								if(get_message(ipc_id_figli, &msg_r, getpid()) < 0)//attendo il segnale di partenza dal gestore
									printf("\nUghino B %d, Error getting message from IPC: %d\n", getpid(), ipc_id_figli);
								figlio_exec(argv[2], ipc_id, ipc_id_figli, sm_pid, sm_wid, popolazione[elim].tipo, settings->init_people);
								break;
							default:
								fantozzi = popolazione[elim].semid;
								popolazione[elim] = crea_ughino_primordiale(stats, settings->genes, pid);
								popolazione[elim].semid = fantozzi;
								printf("\nGestore: Ughino creato:");
								show_ugo(popolazione[elim]);
								//MANDO LA PARTENZA AI FIGLI
								msg_r.mtype = pid;
								sprintf(msg_r.mtext, "GO");
								if(send_message(ipc_id_figli, &msg_r) < 0)
									printf("\nGestore: Error sending message to ughino %d\n", popolazione[cont].pid);
								break;
						}
					}
				}
			}
			while(flag_accoppiamento > 1){
				//eseguo operazioni sul canale ipc per leggere i dati dei processi accoppiati
				//RISPOSTA DAL PROCESSO A
				if(get_message(ipc_id, &msg_r, 0) < 0)
					printf("\nGestore: Error getting message from A\n");
				else{
					flag_accoppiamento--;
					//RISPOSTA DAL PROCESSO B
					if(get_message(ipc_id, &msg_r, atoi(msg_r.mtext)) < 0)
						printf("\nGestore: Error getting message from B");
					else{
						flag_accoppiamento--;
						int elim[2]={0,0};
						int jflag = 0;
						int fantozzi = 0;						
						//prendo la posizione dei processi genitori
						elim[0] = getpos_bypid(popolazione, settings->init_people, atoi(msg_r.mtext));
						elim[1] = getpos_bypid(popolazione, settings->init_people, (int)msg_r.mtype);
						popolazione[elim[0]].stato = STATO_UNUSED;
						popolazione[elim[1]].stato = STATO_UNUSED;
						printf("\n__________________________________________________\nGestore: Uccisione genitori...\n");
						printf("\nGestore: Ughino ucciso:");
						show_ugo(popolazione[elim[0]]);
						printf("\nGestore: Ughino ucciso:");
						show_ugo(popolazione[elim[1]]);
						int mcdgenes = MCD(popolazione[elim[0]].genoma, popolazione[elim[1]].genoma);
						//mi preparo e creare i figli, inserendoli nelle posizioni dei genitori
						for(jflag = 0; jflag < 2; jflag++){
							pid = fork();
							switch(pid){
								case -1:
									printf("\nGestore: ERROR CREATING UGHINO :C\n");
									break;
								case 0:
									if(get_message(ipc_id_figli, &msg_r, getpid()) < 0)//attendo il segnale di partenza dal gestore
										printf("\nUghino B %d, Error getting message from IPC: %d\n", getpid(), ipc_id_figli);
									figlio_exec(argv[2], ipc_id, ipc_id_figli, sm_pid, sm_wid, popolazione[elim[jflag]].tipo, settings->init_people);
									break;
								default:
									printf("\nGestore: Ughino creato:");
									fantozzi = popolazione[elim[jflag]].semid;
									stats->n_processi[popolazione[elim[jflag]].tipo]--;
									popolazione[elim[jflag]] = crea_ughino_non_primordiale(stats, settings->genes, pid, mcdgenes, popolazione[elim[jflag]].nome);
									popolazione[elim[jflag]].semid = fantozzi;
									show_ugo(popolazione[elim[jflag]]);
									//MANDO LA PARTENZA AI FIGLI
									msg_r.mtype = pid;
									sprintf(msg_r.mtext, "GO");
									if(send_message(ipc_id_figli, &msg_r) < 0)
										printf("\nGestore: Error sending message to ughino %d\n", popolazione[cont].pid);
									break;
							}
						}
						printf("\n__________________________________________________\n");
					}
				}
			}
			//RISVEGLIO I FIGLI IN ATTESA
			for(cont = 0; cont < settings->init_people; cont++){
				if(waiters[cont] == 1){
					waiters[cont] = 0;
					kill(popolazione[cont].pid, SIGCONT);
				}
			}
			//dormo per un secondo per tener traccia del tempo passato
			tosleep.tv_sec = 1;
			tosleep.tv_nsec = 0;
			remaining.tv_sec = 0;
			remaining.tv_nsec = 0;
			do{
				retsleep = nanosleep(&tosleep, &remaining);
				tosleep.tv_sec=remaining.tv_sec;
				tosleep.tv_nsec=remaining.tv_nsec;
			}while(retsleep < 0);
			now_time++;
			bd_time--;
		}while(now_time < settings->sim_time);
		//!FORZO LA CHIUSURA DI TUTTI I PROCESSI ANCORA APERTI
		printf("\n_____________________________________________________________________");
		printf("\n_____________________________________________________________________\n");
		printf("\nDeallocazione risorse e terminazione programma.\n\n");
		for(cont = 0; cont < settings->init_people; cont++){
			destroy_semaphore(popolazione[cont].semid);
			if(popolazione[cont].stato != STATO_UNUSED){
				if(kill(popolazione[cont].pid, SIGTERM) < 0)
					printf("\nGestore: ERROR CLOSING UGHINO %d", popolazione[cont].pid);
				else{
					waitpid(popolazione[cont].pid, NULL, 0);
					printf("\nGestore: CLOSING UGHINO %d", popolazione[cont].pid);
					popolazione[cont].stato = STATO_UNUSED;
				}
			}
		}
		
		//!TERMINE PROGRAMMA
		printf("\n\n	STATISTICHE FINALI SULLA POPOLAZIONE:");
		show_stats(stats);
		
		//!DEALLOCAZIONE DELLE RISORSE
		close_ipc_channel(ipc_id);
		close_ipc_channel(ipc_id_figli);
		detach_shared_memory(popolazione);
		detach_shared_memory(waiters);
		close_shared_memory(sm_pid);
		close_shared_memory(sm_wid);
		free(settings);
		free(stats);
		printf("\n\n");
		exit(0);
	} else
		jexit("Per la corretta esecuzione del programma bisogna invocarlo con 3 parametri:\nnome_programma_padre percorso_settings nome_programma_figlio\nIl file settings di base si chiama settings.txt");
}
/*
	Metodo che fa partire il processo figlio, passandogli in argv tutti i dati di cui necessita
*/
void figlio_exec(char* argv2, int ipc_id, int ipc_id2, int sm_pid, int sm_wid, int jtipo, int n_proc){
	char n_p[(126 + 1)]; //PRIMO PARAMETRO DI EXECVE PERMETTE PATH DI MAX 127 CARATTERI
	sprintf(n_p, "./%s", argv2);	
	/*strcpy(n_p, "./");
	strcat(n_p, argv2);*/
	char* argv[] = {argv2, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	char* envp[] = {NULL};
	argv[1] = (char*) malloc(sizeof(int));//mkey del channel di comunicazione requests del gestore
	argv[2] = (char*) malloc(sizeof(int));//mkey del channel di comunicazione requests dei figli
	argv[3] = (char*) malloc(sizeof(int));//mkey della memoria condivisa dei processi
	argv[4] = (char*) malloc(sizeof(int));//mkey della memoria condivisa dei waiters
	argv[5] = (char*) malloc(sizeof(int));//tipo di processo
	argv[6] = (char*) malloc(sizeof(int));//numero processi
	sprintf(argv[1], "%d", ipc_id);//valore ipc_id_gestore
	sprintf(argv[2], "%d", ipc_id2);//valore ipc_id_processi
	sprintf(argv[3], "%d", sm_pid);//valore shared_memory_processi
	sprintf(argv[4], "%d", sm_wid);//valore shared_memory_waiters
	sprintf(argv[5], "%d", jtipo);//valore tipo processo
	sprintf(argv[6], "%d", n_proc);//grandezza popolazione
	execve(n_p, argv, envp);
}
/*
	Metodo che mi ritorna la posizione di un processo disponibile ad essere ucciso(non in fase di creazione o di accoppiamento)
*/
int get_random_processo(jughino* popolazione, int n_max){
	int start = rand() % n_max;
	int res = -1;
	int cicli = 0;
	//cerco un processo da uccidere
	for(res = start; res <= n_max; res++){
		if(res == n_max){//se arrivo alla fine della lista, ricontrollo da 0
			res = 0;
			cicli++;
		}
		if(cicli > 2){ //vuol dire che tutti i processi stanno già morendo di loro
			res = -1;
			break;
		}
		if((popolazione[res].stato == STATO_ATTESA || (popolazione[res].tipo == 0 && popolazione[res].stato == STATO_CHECKING)) && check_and_lock_semaphore(popolazione[res].semid) != -1)
			break;
	}
	return res;
}
//HANDLER DEI SEGNALI
void jsignal_handler(int jsig){
	switch(jsig){
		case SIGUSR1://UN PROCESSO FIGLIO SI è ACCOPPIATO
			flag_accoppiamento++;
			break;
		case SIGUSR2://UN PROCESSO FIGLIO SI è ACCOPPIATO
			flag_accoppiamento++;
			break;
	}
}
