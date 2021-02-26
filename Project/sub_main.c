#include "jpreferences.h"

void pulizia();
void pulizia_coda_ipc_a(pid_t);
void pulizia_coda_ipc_b(pid_t);
void processo_A(pid_t, int, int);
void processo_B(pid_t, int, int);
void jsignal_handler_a(int);
void jsignal_handler_b(int);

jughino* popolazione;
int* waiters;
int ipc_id_p;
struct timespec tosleep;

int main(int argc, char* argv[]){
	if(argc == 7){
		pid_t pid = getpid();
		//printf("\nMyPid=%d myType=%d", pid,atoi(argv[6]));
		int ipc_id_g = atoi(argv[1]);
		ipc_id_p = atoi(argv[2]);
		int sm_pid = atoi(argv[3]);
		int sm_wid = atoi(argv[4]);
		int ughini_max = atoi(argv[6]);
		popolazione = (jughino*)shmat(sm_pid, NULL, 0);
		waiters = (int*)shmat(sm_wid, NULL, 0);		
		if(atoi(argv[5]) == 0) { //tipo A
			signal(SIGTERM, jsignal_handler_a);
			//printf("\nUghino %d, ipc_id: %d, shared_id: %d, shared_sem: %d, tipo A\n", pid, ipc_id, sm_id, sm_sem);
			processo_A(pid, ipc_id_g, ughini_max);
			pulizia_coda_ipc_a(pid);
		} else { //tipo B
			signal(SIGTERM, jsignal_handler_b);
			//printf("\nUghino %d, ipc_id: %d, shared_id: %d, shared_sem: %d, tipo B\n", pid, ipc_id, sm_id, sm_sem);
			processo_B(pid, ipc_id_g, ughini_max);
			pulizia_coda_ipc_b(pid);
		}
		pulizia();
		exit(0);
	} else
		jexit("Errore nel numero parametri.\neseguibile ipc_id ipc_id2 sm_pid sm_wid sm_sem tipo num_ughini");
}

void processo_A(pid_t pid, int ipc_id_g, int n_max){
	int keep = 1;
	int pos = getpos_bypid(popolazione, n_max, pid);
	int mytarget = popolazione[pos].genoma;
	int richieste[n_max];
	int cont, update = 0;
	jmessage msg;
	popolazione[pos].stato = STATO_ATTESA;
	for(cont = 0; cont < n_max; cont++)
		richieste[cont] = -1;
	//ciclo finchè non trovo qualche processo con cui accoppiarmi
	do {
		msg.mtype = pid;
		if(get_message(ipc_id_p, &msg, pid) < 0)
			printf("\nUghino A %d, Error getting REQUEST MESSAGE from IPC: %d", pid, ipc_id_p);
		else{
			//blocco mio semaforo
			while(check_and_lock_semaphore(popolazione[pos].semid) == -1){
				waiters[pos] = 1;
				if(kill(pid, SIGSTOP) < 0)
					printf("\nUghino B %d, Error putting myself to sleep at 1st sem\n", pid);
			}
			popolazione[pos].stato = STATO_CHECKING;
			int pos_other = getpos_bypid(popolazione, n_max, atoi(msg.mtext));
			jughino other = popolazione[pos_other];
			/*printf("\nUghino A %d, genoma = %ld, other.genoma = %ld, mcd = %d, target = %d\n", pid, popolazione[pos].genoma, other.genoma, MCD(popolazione[pos].genoma, other.genoma) ,mytarget);*/
			if(other.genoma % popolazione[pos].genoma == 0 || MCD(popolazione[pos].genoma, other.genoma) > mytarget){//se mi va bene per me
				popolazione[pos].stato = STATO_ACCOPPIANDO;
				msg.mtype = other.pid;
				sprintf(msg.mtext, "%d", ANSWER_OK); //DICO OK AL PROCESSO
				if(send_message(ipc_id_p, &msg) < 0)
					printf("\nUghino A %d, Error sending ANSWER_OK in IPC: %d", pid, ipc_id_p);
				else{
					keep = 0;
					msg.mtype = pid;
					sprintf(msg.mtext, "%d", other.pid); //DICO OK AL GESTORE DICENDOGLI CON CHI MI ACCOPPIO
					if(send_message(ipc_id_g, &msg) < 0)
						printf("\nUghino A %d, Error sending message to GESTORE in IPC: %d", pid, ipc_id_g);

					else if(kill(getppid(), SIGUSR1) < 0)
						printf("\nUghino A %d, Error SIGNALING GESTORE ABOUT NEW MESSAGE\n", pid);
				}				
			} else { //se mi va bene
				popolazione[pos].stato = STATO_ATTESA;
				//aggiorno requests
				richieste[pos_other] = popolazione[pos_other].pid;
				update = 0;
				for(cont = 0; cont < n_max; cont++)
					if(cont != pos && (popolazione[cont].tipo == 1 && popolazione[cont].stato != STATO_CREATO && richieste[cont] != popolazione[cont].pid))
						update = 1;
				if(update == 0)
					mytarget-= (popolazione[pos].genoma/25);//riduco del 25% il mio target
				mytarget--;
				msg.mtype = other.pid;
				sprintf(msg.mtext, "%d", ANSWER_NOPE); //DICO NO AL PROCESSO
				if(send_message(ipc_id_p, &msg) < 0)
					printf("\nUghino A %d, Error sending ANSWER_NOPE in IPC: %d", pid, ipc_id_p);
			}
			if(keep == 1){//se devo aspettare prossima richiesta
		        popolazione[pos].stato = STATO_ATTESA;
				update_semaphore(popolazione[pos].semid, 1);
				//imposto un tempo random da 0.1-0.3 di dormita
				tosleep.tv_sec = 0;
				tosleep.tv_nsec = (100000000ULL * rand() / RAND_MAX);
				nanosleep(&tosleep, NULL);
			}else //sblocco mio semaforo
				update_semaphore(popolazione[pos].semid, 1);
		}
	} while(keep == 1);
}
void processo_B(pid_t pid, int ipc_id_g, int n_max){
	int keep = 1;
	int mytarget = rand() % n_max;
	int pos = getpos_bypid(popolazione, n_max, pid);
	jmessage msg;
	popolazione[pos].stato = STATO_ATTESA;
	do {
		//blocco il mio semaforo
		while(check_and_lock_semaphore(popolazione[pos].semid) == -1){
			waiters[pos] = 1;
			//printf("\nWaiters %d set to %d process B",pos,1);
			if(kill(pid, SIGSTOP) < 0)
				printf("\nUghino B %d, Error putting myself to sleep at 1st sem\n", pid);
		}
		if(popolazione[mytarget].tipo == 0 && popolazione[mytarget].stato != STATO_ACCOPPIANDO && popolazione[mytarget].stato != STATO_CREATO){
			popolazione[pos].stato = STATO_CHECKING;
			msg.mtype = popolazione[mytarget].pid;
			sprintf(msg.mtext, "%d", pid);
			if(send_message(ipc_id_p, &msg) < 0)
				printf("\nUghino B %d, Error sending message in IPC: %d\n", pid, ipc_id_p);
			else{
				msg.mtype = pid;
				if(get_message(ipc_id_p, &msg, pid) < 0)
					printf("\nUghino B %d, Error getting message from IPC: %d\n", pid, ipc_id_p);
				else{
					if(atoi(msg.mtext) == ANSWER_OK){
						//printf("\nSono B %d e ho ricevuto COMEOVERHERE da A %d\n",pid,popolazione[mytarget].pid);
						popolazione[pos].stato = STATO_ACCOPPIANDO;
						keep = 0;
						msg.mtype = pid;
						sprintf(msg.mtext, "%d", popolazione[mytarget].pid); //DICO OK AL GESTORE DICENDOGLI CON CHI MI ACCOPPIO
						if(send_message(ipc_id_g, &msg) < 0)
							printf("\nUghino B %d, Error sending message to GESTORE in IPC: %d\n", pid, ipc_id_g);
						else if(kill(getppid(), SIGUSR2) < 0)
							printf("\nUghino B %d, Error SIGNALING GESTORE ABOUT NEW MESSAGE\n", pid);
					}
				}
			}
		}
		//sblocco mio semaforo
		if(keep == 1){//mi preparo per la prossima richiesta
			popolazione[pos].stato = STATO_ATTESA;
			update_semaphore(popolazione[pos].semid, 1);
			tosleep.tv_sec = 0;
			tosleep.tv_nsec = (100000000ULL * rand() / RAND_MAX);
			nanosleep(&tosleep, NULL);
		}else
			update_semaphore(popolazione[pos].semid, 1);
		//faccio ciclare nell'array degli ughini
		while((popolazione[mytarget].tipo != 0 || popolazione[mytarget].stato == STATO_ACCOPPIANDO) && mytarget < n_max){
		        mytarget++;    
		}
		if(mytarget == n_max)
		     mytarget = 0;
	} while(keep == 1);
}

//METODI DI PULIZIA PRE-KILL
void pulizia(){
	detach_shared_memory(popolazione);
	detach_shared_memory(waiters);
}
void pulizia_coda_ipc_a(pid_t pid){
	sleep(1); //attendo i possibili nuovi messaggi che arrivino
	jmessage temp;
	int keep = 0;
	do{
		temp.mtype = pid;
		keep = clean_message(ipc_id_p, &temp, pid);
		if(keep == 0) {//c'è un messaggio
			temp.mtype = atoi(temp.mtext);
			sprintf(temp.mtext, "%d", ANSWER_NOPE);
			if(send_message(ipc_id_p, &temp) < 0)
					printf("\nUghino A %d, Error sending message in CLEANING IPC: %d\n", pid, ipc_id_p);
		}
	}while(keep == 0);
}
void pulizia_coda_ipc_b(pid_t pid){
	sleep(1); //attendo i possibili nuovi messaggi che arrivino
	jmessage temp;
	int keep = 0;
	do{
		temp.mtype = pid;
		keep = clean_message(ipc_id_p, &temp, pid);
	}while(keep == 0);
}
//HANDLER DEI SEGNALI
void jsignal_handler_a(int jsig){
	switch(jsig){
		case SIGTERM: //IL PADRE MI VUOLE MORTO
			//pulizia varia e terminazione
			pulizia();
			pulizia_coda_ipc_a(getpid());
			exit(1);
			break;
	}
}

void jsignal_handler_b(int jsig){
	switch(jsig){
		case SIGTERM: //IL PADRE MI VUOLE MORTO
			//pulizia varia e terminazione
			pulizia();
			pulizia_coda_ipc_b(getpid());
			exit(1);
			break;
	}
}
