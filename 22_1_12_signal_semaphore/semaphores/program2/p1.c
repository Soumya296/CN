#include <semaphore.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    key_t key1=ftok("key1",65),key2=ftok("key2",66);

    int shmid1=shmget(key1,1024,0666|IPC_CREAT);
    int shmid2=shmget(key2,1024,0666|IPC_CREAT);

    int* x=(int*)shmat(shmid1,(void*)0,0);
    int* y=(int*)shmat(shmid2,(void*)0,0);

    sem_t *s1=sem_open("s1",O_CREAT,0666,0);
    sem_t *s2=sem_open("s2",O_CREAT,0666,0);

    *x=1;
    *y=1;

    while(1)
    {
        printf("I am reading shm y \n");
        *x=*y+1;
        printf("\nValue of y %d\n",*y);
        printf("value of x after incrementing y : %d\n",*x);
        printf("Enter any character to signal S1\n");
        char buff[3];
        fgets(buff,3,stdin);
        sem_post(s1);
        printf("I am waiting for S2...\n");
        sem_wait(s2);
    }
}