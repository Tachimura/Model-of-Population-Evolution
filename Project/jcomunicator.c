#include "jcomunicator.h"
/*
	Metodi per:
	SPEDIRE, RICEVERE, MESSAGGI
	PULIRE LA CODA DI MESSAGGI
*/
int send_message(int ipcid, jmessage* msg){
	//printf("\nsend message pid= %d",atoi(msg->mtext));
	return msgsnd(ipcid, msg, (sizeof(jmessage) - sizeof(long)), 0);
}
int get_message(int ipcid, jmessage* msg, int mtype){
	//printf("\nwaiting for message");
	return msgrcv(ipcid, msg, (sizeof(jmessage) - sizeof(long)), mtype, 0);
}
int clean_message(int ipcid, jmessage* msg, int mtype){
	if(msgrcv(ipcid, msg, (sizeof(jmessage) - sizeof(long)), mtype, IPC_NOWAIT) == -1){
		if(errno == ENOMSG)
			return 1;
		else
			return -1;
	}else
		return 0;
}
/*
	Metodi per:
	Gestione di creazione/eliminazione canali ipc/shared memory e semafori
*/
int open_ipc_channel(){
	return msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IXUSR);
}
int close_ipc_channel(int ipcid){
	int res = msgctl(ipcid, IPC_RMID, NULL);//chiudo coda messaggi
	if(res < 0)
		return res;
	return shmctl(ipcid, IPC_RMID, NULL);//chiudo ipc channel
}

int create_semaphore(){
	int semid = 0;
	semid = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR | S_IXUSR);
	return set_semaphore(semid, 1);
}
/*
	Operazione di lettura e settaggio di un semaforo
*/
int check_and_lock_semaphore(int semid){
	struct sembuf sop;
	//istruzione di check and set
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = IPC_NOWAIT;
	return semop(semid, &sop, 1);
}
int update_semaphore(int semid, int val){
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = val;
	sop.sem_flg = 0;
	return semop(semid, &sop, 1);
}
int set_semaphore(int semid, int val){
	union semun arg;
	arg.val = val;
	if(semctl(semid, 0, SETVAL, arg) < 0)
		return -1;
	return semid;
}
int get_semaphore_val(int semid){
	return semctl(semid, 0, GETVAL, NULL);
}
int create_shared_memory(int dimension){
	return shmget(IPC_PRIVATE, dimension, S_IRUSR | S_IWUSR | S_IXUSR);
}
int detach_shared_memory(const void* addr){
	return shmdt(addr);
}
void destroy_semaphore(int sem_id){
	semctl(sem_id, 0, IPC_RMID, NULL);
}
void close_shared_memory(int sm_id){
	shmctl(sm_id, IPC_RMID, 0);
}
/*
	Metodo di controllo generico di errori
*/
void jexit(char * errore){
	printf("\nProgram Error: %s\n", errore);
	exit(-1);
}
