#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<pthread.h>

const char * sem_12 = "sem_12";
const char * sem_41 = "sem_41";

int main()
{
    sem_t * sem12 = sem_open(sem_12, O_CREAT, 0666, 0);
    sem_t * sem41 = sem_open(sem_41, O_CREAT, 0666, 0);

    printf("\nI am P1. Enter anything to signal semaphore 12\n");
    
    char ch;
    scanf("%s",&ch);
    if(ch == '\n') return 1;

    sem_post(sem12);
    printf("Signalling semaphore 12\n");

    
    printf("Waiting for semaphore 41\n");
    sem_wait(sem41);

    printf("Got semaphore signalling for 41\n");
    sem_post(sem41);

    // sem_close(sem41);
}