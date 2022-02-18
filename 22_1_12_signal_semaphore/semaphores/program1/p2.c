#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<pthread.h>

const char * sem_12 = "sem_12";
const char * sem_23 = "sem_23";

int main()
{
    sem_t * sem12 = sem_open(sem_12, O_CREAT, 0666, 0);
    sem_t * sem23 = sem_open(sem_23, O_CREAT, 0666, 0);

    printf("\nI am P2. waiting for semaphore 12\n");
    sem_wait(sem12);
    printf("Got semaphore signalling from P1\n");
    sem_post(sem12);

    printf("\nI am P2. Enter anything to signal semaphore 23\n");
    
    char ch;
    scanf("%s",&ch);
    if(ch == '\n') return 1;

    sem_post(sem23);
    printf("Signalling semaphore 23\n");

    // sem_close(sem12);    
}