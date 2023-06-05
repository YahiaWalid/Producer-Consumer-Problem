#ifndef YAHIA_SEMAPHORE_H_INCLUDED
#define YAHIA_SEMAPHORE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


union semun{
     int val;				    // value for SETVAL
     struct semid_ds *buf;		//buffer for IPC_STAT & IPC_SET
     unsigned short int *array;	// array for GETALL & SETALL
     struct seminfo *__buf;		// buffer for IPC_INFO
   };


void semWait (int semid)
{
    struct sembuf sops;

    sops.sem_num  = 0;      //semaphore number  in our case 0 bec it is a one elemement semaphore that we initialized
    sops.sem_op   = -1;     //semaphore operation -1 --> wait
    sops.sem_flg = 0;       //operation flag

    if (semop(semid, &sops, 1)== -1) {
	    perror ("semop: wait");
	    exit (1);
        }
}

void semSignal (int semid)  //not done
{
    struct sembuf sops;

    sops.sem_num  = 0;  //semaphore number
    sops.sem_op   = 1;  //semaphore 1 --> signal
    sops.sem_flg = 0;   //operation flag

    if (semop(semid, &sops, 1) == -1) {
	    perror ("semop: signal");
        exit (1);
        }
}


#endif // YAHIA_SEMAPHORE_H_INCLUDED
