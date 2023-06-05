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




//struct of commodities to hold their info
struct commodity {
  int  id;
  char name[10];
  double mean;
  double variance;
  int sleepTime;
  double lastAverage;
  deque <double> last5prices;

};

//struct item which represents an item which is to be placed on or read from the buffer
struct item{

    char name[11];  //holds name of producer who put the item
    double price;   //price of item

};


struct shared_mem{

        int next_in=0;             //pointer to next price to be placed on buffer
        int next_out=0;            //pointer to next price to be read from buffer
        int numberOfProducers=0;   //count used to reject any producers in the future after number reaches 20
        item buffer[];             //buffer represented as an array of struct items which contains name and price
};

//function used to get the average of the deque
double getAverageOfDeque(deque <double> v){

    if(v.size()==0)
        return 0;
    double sum=0;
    for(int i=0;i<v.size();i++)
        sum+=v[i];

    return sum/v.size();
}


//assuming commodities have ids from 1 to 11
int getID(char c_name[]){

    if(strcmp(c_name,"ALUMINIUM")==0)
        return 1;
    else if(strcmp(c_name,"COPPER")==0)
        return 2;
    else if(strcmp(c_name,"COTTON")==0)
        return 3;
    else if(strcmp(c_name,"CRUDEOIL")==0)
        return 4;
    else if(strcmp(c_name,"GOLD")==0)
        return 5;
    else if(strcmp(c_name,"LEAD")==0)
        return 6;
    else if(strcmp(c_name,"MENTHAOIL")==0)
        return 7;
    else if(strcmp(c_name,"NATURALGAS")==0)
        return 8;
    else if(strcmp(c_name,"NICKEL")==0)
        return 9;
    else if(strcmp(c_name,"SILVER")==0)
        return 10;
    else if(strcmp(c_name,"ZINC")==0)
        return 11;
    else return -1;
}

commodity initialize(char *name){

    commodity c;
    strcpy(c.name,name);
    c.id =getID(name);              //assigning the IDs to the commodities
    c.last5prices.push_back(0.00);  //first price for each commodity   is 0.00
    c.lastAverage=0.00;             //first average for each commodity is 0.00
    return c;
}

// function modifies the table according to the new prices
// in third and fifth parameters , 1 indicates inc , 0 indicates decrease , -1 indicates no change
void modify_table(char c_name[],double price,int price_inc,double average,int average_inc){ // assuming first commodity has id 1 and so on

    int id = getID(c_name);
    if (id == -1) return;
    int row = id + 3;   //row id + 3 contains commodity bec first three lines printed dont have commodities

    //price
    printf("\033[%d;19H",row); //moving print cursor to price position
    if(price_inc==1) printf("\033[;32m%7.2f↑\033[0m",price); //increase print
    else if(price_inc==0) printf("\033[;31m%7.2f↓\033[0m",price); //decrease print
    else if(price_inc==-1) printf("\033[;34m%7.2f\033[0m",price); //no change
    printf("\033[16;1H"); //moving print cursor to end
    fflush(stdout);
    printf("\033[16;1H"); //moving print cursor to end

    //avg
    printf("\033[%d;30H",row); //moving print cursor to avg position
    if(average_inc==1) printf("\033[;32m%7.2f↑\033[0m",average); //increase print
    else if(average_inc==0)printf("\033[;31m%7.2f↓\033[0m",average); //decrease print
    else if(average_inc==-1) printf("\033[;34m%7.2f\033[0m",average); //no change
    printf("\033[16;1H"); //moving print cursor to end
    fflush(stdout);
    printf("\033[16;1H"); //moving print cursor to end
}

void printZerosTable(char names[11][11]){

    //initializing table to zeros
    float v1 =0;
    printf("\e[1;1H\e[2J"); //clearing screen first
    printf("+-------------------------------------+\n");
    printf("| Currency     |    Price | AvgPrice  |\n");
    printf("+-------------------------------------+\n");

    for(int i=0;i<11;i++){
        printf("| %-10s   |  \033[;34m%7.2f\033[0m |  \033[;34m%7.2f\033[0m  |\n",names[i],v1,v1);
    }
    printf("+-------------------------------------+\n");
    printf("\033[16;1H"); //moving print cursor to end
}


int main(int argc,char *argv[])
{


    int n_buffer ;   //buffer size

    if(argc !=2){

        printf("Please enter buffer size\n");
        scanf("%d",&n_buffer);

    }
    else{

        n_buffer = atoi(argv[1]);
    }


    char table_names[11][11]={"ALUMINIUM","COPPER","COTTON","CRUDEOIL","GOLD","LEAD","MENTHAOIL","NATURALGAS","NICKEL","SILVER","ZINC"};
    commodity c_array[11];

    //printf("reached here\n");
    for(int i =0;i<11;i++)
        c_array[i]=initialize(table_names[i]);


    printZerosTable(table_names);

    shared_mem* shared_memory;



    int shmid;

    key_t key ,key_e , key_s , key_n;

    //generating shared memory key

    if ((key =  ftok("Makefile", 'E')) == -1) {
        perror ("ftok");
        exit(1);
    }


    //generating 3 keys for the 3 semaphores

    if ((key_e =  ftok("sem_e", 'E')) == -1) {
        perror ("ftok");
        exit(1);
    }
    if ((key_s =  ftok("sem_s", 'E')) == -1) {
        perror ("ftok");
        exit(1);
    }
    if ((key_n =  ftok("sem_n", 'E')) == -1) {
        perror ("ftok");
        exit(1);
    }



    int sem_e,sem_s,sem_n;

    if((sem_e = semget(key_e, 1, 0666 | IPC_CREAT))==-1){        // creating e(empty) semaphore
        perror("semget1");
        exit(1);
        }

    if((sem_s= semget(key_s, 1, 0666 | IPC_CREAT))==-1){           // creating s(mutex)  semaphore
        perror("semget2");
        exit(1);
        }

    if((sem_n= semget(key_n, 1, 0666 | IPC_CREAT))==-1){           // creating n(full)  semaphore
        perror("semget3");
        exit(1);
        }



    union semun s;

    s.val = n_buffer;
    if (semctl (sem_e, 0, SETVAL, s) == -1) {               // initializing sem_e(empty) to n_buffer
        perror ("semctl1 SETVAL");
        exit(1);
    }

    s.val = 1;
    if (semctl (sem_s, 0, SETVAL, s) == -1) {               // initializing sem_s(mutex) to 1
        perror ("semctl2 SETVAL");
        exit(1);
    }
    s.val=0;
    if (semctl (sem_n, 0, SETVAL, 0) == -1) {               // initializing sem_n(empty) to 0
        perror ("semctl3 SETVAL");
        exit(1);
    }




    //shmid=shmget((key_t)2345,n_buffer *sizeof(item) + sizeof(shared_mem),0666 | IPC_CREAT);
    shmid=shmget(key,sizeof(shared_mem) + n_buffer*sizeof(item),0666 | IPC_CREAT);
    //printf("Key of shared memory is %d\n",shmid);
    shared_memory=(shared_mem*)shmat(shmid,NULL,0);//process attached to shared memory segment 



    //clearing memory in case it was not cleared before
        shmdt(shared_memory);
        shmctl(shmid,IPC_RMID,NULL);
        shmid=shmget(key,sizeof(shared_mem) + n_buffer*sizeof(item),0666 | IPC_CREAT);
        shared_memory=(shared_mem*)shmat(shmid,NULL,0);



    double new_average;



    while(true){

        //printf("\033[16;1H"); //moving print cursor to end


        char c_name[11];


        semWait(sem_n);     //Waiting on n  {full}
        semWait(sem_s);     //waiting on s  {mutex}
        /////////////////////////////////////// BEGIN CRITICAL //////////////////////////////////////////////////////////
        strcpy(c_name,shared_memory->buffer[shared_memory->next_out].name);
        double new_price = shared_memory->buffer[shared_memory->next_out].price;
        shared_memory->next_out= (shared_memory->next_out + 1) % n_buffer;
        //////////////////////////////////////// END CRITICAL ///////////////////////////////////////////////////////////
        semSignal(sem_s);    //signalling s  {mutex}
        semSignal(sem_e);    //signalling n  {empty}



        //consuming

        int temp_id  = getID(c_name);
        int price_inc;
        int avg_inc;


        if (c_array[temp_id-1].last5prices.size() == 6)
            c_array[temp_id-1].last5prices.pop_front();




        try
        {
            new_average=getAverageOfDeque(c_array[temp_id-1].last5prices);
        }
        catch (std::bad_alloc & exception)
        {
            std::cerr << "bad_alloc detected: " << exception.what() << "\n";
        }


        if(new_average > c_array[temp_id-1].lastAverage )
            avg_inc=1;  //increase
        else if (new_average < c_array[temp_id-1].lastAverage )
            avg_inc=0;  //decrease
        else
            avg_inc=-1; //no change


        c_array[temp_id-1].lastAverage = new_average;
        if(c_array[temp_id-1].last5prices.size()>0){

            double lp = c_array[temp_id-1].last5prices.back();  //last price

            if(new_price > lp )
                price_inc=1;    //inc
            else if(new_price < lp )
                price_inc=0;    //dec
            else
                price_inc=-1;   //nc
        }

        else
            price_inc=1;       //first price after initial 0.00 price will obv be inc

        c_array[temp_id-1].last5prices.push_back(new_price);
        modify_table(c_name,new_price,price_inc,new_average,avg_inc);

    }


    printf("\033[16;1H");  //moving print cursor to end


    //shmdt(shared_memory);

    if(shmctl(shmid,IPC_RMID,NULL)==-1){
        perror ("semctl remove:");
        exit(1);
    }


    if (semctl (sem_e, 0, IPC_RMID) == -1) {               // clearing sem_e
        perror ("semct1 clear");
        exit(1);
    }
    if (semctl (sem_s, 0, IPC_RMID) == -1) {               // clearing sem_s
        perror ("semctl2 clear");
        exit(1);
    }
    if (semctl (sem_n, 0, IPC_RMID) == -1) {               // clearing sem_n
        perror ("semctl3 clear");
        exit(1);
    }

    printf("cleared memory/semaphores and exiting\n");
    printf("\033[17;1H"); //moving print cursor to end

return 0;
}
