#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<semaphore.h>
#include<sys/types.h>
#include<pthread.h>

const char * sem_23 = "sem_23";
const char * sem_34 = "sem_34";

int main()
{
    sem_t * sem23 = sem_open(sem_23, O_CREAT, 0666, 0);
    sem_t * sem34 = sem_open(sem_34, O_CREAT, 0666, 0);

    printf("\nI am P3. waiting for semaphore 23\n");
    sem_wait(sem23);
    printf("Got semaphore signalling from P2\n");
    sem_post(sem23);

    printf("\nI am P3. Enter anything to signal semaphore 34\n");
    
    char ch;
    scanf("%s",&ch);
    if(ch == '\n') return 1;

    sem_post(sem34);
    printf("Signalling semaphore 34\n");

    // sem_close(sem23);
}