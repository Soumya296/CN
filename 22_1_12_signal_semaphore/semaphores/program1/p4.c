#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<pthread.h>

const char * sem_34 = "sem_34";
const char * sem_41 = "sem_41";

int main()
{
    sem_t * sem34 = sem_open(sem_34, O_CREAT, 0666, 0);
    sem_t * sem41 = sem_open(sem_41, O_CREAT, 0666, 0);

    printf("\nI am P4. waiting for semaphore 34\n");
    sem_wait(sem34);
    printf("Got semaphore signalling from P3\n");
    sem_post(sem34);

    printf("\nI am P4. Enter anything to signal semaphore 41\n");
    
    char ch;
    scanf("%s",&ch);
    if(ch == '\n') return 1;

    sem_post(sem41);
    printf("Signalling semaphore 41\n");

    // sem_close(sem34);
}