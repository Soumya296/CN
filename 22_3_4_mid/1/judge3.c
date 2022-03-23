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

/*Global variables for signal actuation*/
int WAIT_CHOOSE = 0;
int WAIT_PUT = 0;

void handler_USR1(int sifid, siginfo_t *info, void *context)
{
    WAIT_CHOOSE = 1;
}

void handler_USR2(int sifid, siginfo_t *info, void *context)
{
    WAIT_PUT = 1;
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

    while(WAIT_CHOOSE == 0 && WAIT_PUT == 0) {}

    /*PID retrieval*/
    int fd = fileno(popen("pidof ./anchor", "r"));
    char s[1024];
    read(fd,&s,1024);
    close(fd);
    pid_t pid_anchor = atoi(s);

    pid_t pid_judge[2];
    fd = fileno(popen("pidof ./judge1", "r"));
    read(fd,&s,1024);
    pid_judge[1] = atoi(s);

    fd = fileno(popen("pidof ./judge2", "r"));
    read(fd,&s,1024);
    pid_judge[1] = atoi(s);

    
    if(WAIT_CHOOSE > 0)
    {
        for(int i=0; i<3;i++)
        {
            message.mtype = getpid();
            sprintf(message.mtext, "random_question\n");
            msgsnd(msqid,&message,sizeof(message.mtext),0);
        }
        message.mtype = getpid();
        sprintf(message.mtext, "done");
        msgsnd(msqid,&message,sizeof(message.mtext),0);
        kill(pid_judge[0],SIGUSR2);
        kill(pid_judge[1],SIGUSR2);
    }

    message.mtype = getpid();
    sprintf(message.mtext, "%d", 15);
    msgsnd(msqid,&message,sizeof(message.mtext),0);

    kill(pid_anchor, SIGUSR2);

}