#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>

#define MAX 1024
#define port 8080

struct msg_buf {
    long mtype;
    char mtext[100];
};

/*global variables for signalling actuation*/
int CHOOSE_JUDGE = 0;
int JUDGE_COUNT = 0;

void handler_USR1(int sifid, siginfo_t *info, void *context)
{
    CHOOSE_JUDGE = 1;
}

void handler_USR2(int sifid, siginfo_t *info, void *context)
{
    JUDGE_COUNT++;
}

int main()
{
    /*HAndling SIGUSR1*/
    struct sigaction act={0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler_USR1;
    sigaction(SIGUSR1,&act,NULL);

    /*Handling SIGUSR2*/
    struct sigaction action={0};
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = &handler_USR2;
    sigaction(SIGUSR2,&action,NULL);

    /*Init of Message Queue*/
    key_t key = ftok("./queue",0);
    int msqid = msgget(key, IPC_CREAT|0666);
    struct msg_buf message;

    /*Getting the pid of anchor to get the judges' Process ID*/
    char * command;
    int fd, pid_judge[3], pid_perf;
    char s[1024];

    for(int i=0; i<3; i++){
        sprintf(command,"pidof ./judge%d",i+1);
        fd = fileno(popen(command, "r"));
        read(fd,&s,1024);
        pid_judge[i] = atoi(s);
    }

    while(CHOOSE_JUDGE==0){}

    fd = fileno(popen("pidof ./performer","r"));
    read(fd,&s,1024);
    pid_perf = atoi(s);
    close(fd);

    /*Choosing Judge Randomly*/
    int r = rand();
    if(r>50)
    {
        r = 0;
    }
    else if(r>100)
    {
        r = 1;
    }
    else r = 2;

    printf("\nJudge%d is chosen\n",r+1);

    sprintf(message.mtext, "%d", pid_judge[r]);
    message.mtype = getpid();
    msgsnd(msqid,&message,sizeof(message.mtext),0);

    kill(pid_perf,SIGUSR1);
    kill(pid_judge[r],SIGUSR1);

    while(JUDGE_COUNT!=3){}
    
    for(int i=0; i<3; i++)
    {
        msgrcv(msqid, &message,sizeof(message.mtext),pid_judge[i], 0);
        printf("Score from Judge %d is: %s\n", i+1,message.mtext);
    }
    return 0;
}