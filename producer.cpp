#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <bits/stdc++.h>
#include <string>
#include <random>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <map>
#include <time.h>
#include <signal.h>
#include <deque>
#include "yahia_semaphore.h"


using namespace std;


struct commodity {
  char name[11];
  double mean;
  double variance;
  int sleepTime;
  double lastAverage;
  deque <double> last5prices;

};

struct item{
    char name[11];
    double price;
};

struct shared_mem{

        int next_in=0;
        int next_out=0;
        int numberOfProducers=0;
        item buffer[];

};

//function returns time formatted string in nano resolution
char* get_current_time(){

    char *time_string = (char *)malloc(50);
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    struct tm *timePointerEnd = localtime(&tv.tv_sec);
    size_t nbytes = strftime(time_string, 100, "%m/%d/%Y %T.", timePointerEnd);
    snprintf(time_string + nbytes, sizeof(time_string) - nbytes,"%.9ld", tv.tv_nsec);
    return time_string;

}

int main(int argc,char *argv[])
{

    //  ./producer  cName      cMean   cVariance cSleepTime (ms)
    //  ./producer  NATURALGAS 7.1     0.5       200


    commodity c;

    char cName[10];                                 //Commodity name  (no more than 10 characters)
    strcpy(cName,argv[1]);
    strcpy(c.name,argv[1]);

    c.mean            = strtod(argv[2],NULL);       //Commodity Price Mean; 𝜇
    c.variance        = strtod(argv[3],NULL);       //Commodity Price Standard Deviation; 𝜎
    c.sleepTime       = atoi(argv[4]);              //Length of the sleep interval in milliseconds
    int microseconds  = c.sleepTime *1000;          //converting milliseconds to microseconds as usleep() function takes microseconds
    int n_buffer      = atoi(argv[5]);

    printf("Commodity name : %s\nCommodity Price Mean : %.2f\nCommodity Price Standard Deviation : %.2f\nSleeping time: %d milliseconds\n",c.name,c.mean,c.variance,c.sleepTime);



    key_t key ,key_e , key_s , key_n;
    //key = ftok("Makefile", 'E');

    if ((key =  ftok("Makefile", 'E')) == -1) {
        perror ("ftok");
    }


    if ((key_e =  ftok("sem_e", 'E')) == -1) {
        perror ("ftok");
    }
    if ((key_s =  ftok("sem_s", 'E')) == -1) {
        perror ("ftok");
    }
    if ((key_n =  ftok("sem_n", 'E')) == -1) {
        perror ("ftok");
    }

    int sem_e,sem_s,sem_n;

    if((sem_e = semget(key_e, 1, 0666 | IPC_CREAT))==-1){           // e(empty) semaphore
        perror("semget");
        exit(1);
        }

    if((sem_s= semget(key_s, 1, 0666 | IPC_CREAT))==-1){           // s(mutex)  semaphore
        perror("semget");
        exit(1);
        }

    if((sem_n= semget(key_n, 1, 0666 | IPC_CREAT))==-1){           // n(full)  semaphore
        perror("semget");
        exit(1);
        }




    int shmid;
    shmid=shmget(key,sizeof(shared_mem) + n_buffer*sizeof(item),0666);

    //printf("Key of shared memory is %d\n",shmid);


    //process attached to shared memory segment 
    shared_mem* shared_memory;

    shared_memory=(shared_mem *)shmat(shmid,NULL,0);


    //random number generator according to a normal distribution
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(c.mean,c.variance);



    if(shared_memory->numberOfProducers ==21){  //rejecting producers after we have had 20 of them
        shared_memory->numberOfProducers--;
        exit(1);
    }




    while(true){



    //procuding
    double new_price = distribution(generator);

    char* current_time = get_current_time();
    fprintf(stderr, "[%s] %s: generating a new value %7.2f\n",current_time, c.name,new_price);
    fflush(stdout);


/*
    char* current_time4 = get_current_time();
    fprintf(stderr, "[%s] %s: waiting on empty\n",current_time4, c.name);
    fflush(stdout);
*/

    semWait(sem_e);     //waiting on e {empty}


    char* current_time2 = get_current_time();
    fprintf(stderr, "[%s] %s: trying to get mutex on shared buffer\n",current_time2, c.name);
    fflush(stdout);


    semWait(sem_s);     //waiting on s {mutex}

    /////////////////////////////////////// BEGIN CRITICAL //////////////////////////////////////////////////////////
    char* current_time3 = get_current_time();
    fprintf(stderr, "[%s] %s: placing %7.2f on shared buffer in slot %d \n",current_time3, c.name,new_price,shared_memory->next_in);
    fflush(stdout);
    strcpy(shared_memory->buffer[shared_memory->next_in].name,c.name);
    shared_memory->buffer[shared_memory->next_in].price=new_price;
    shared_memory->next_in= (shared_memory->next_in + 1) % n_buffer;
    //////////////////////////////////////// END CRITICAL ///////////////////////////////////////////////////////////
    semSignal(sem_s);    //signaling  s {mutex}
    semSignal(sem_n);    //signalling n {full}


    current_time = get_current_time();
    fprintf(stderr, "[%s] %s: sleeping for %d milliseconds\n",current_time, c.name,c.sleepTime);
    fflush(stdout);

    usleep(microseconds);


    }


return 0;
}
