#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/msg.h>
#include<sys/ipc.h>

struct message_buf{
    long mtype;
    char mtext[100];
};

int count_left=0;
int count_right=0;

pid_t left, right;

void handler_USR1(int sifid, siginfo_t *info, void *context)
{
    left = info->si_pid;
    printf("signal sent to p1 from p4 with PID : %d\n", left);
    fflush(stdout);
    count_left++;
}

void handler_USR2(int sifid, siginfo_t *info, void *context)
{
    right = info->si_pid;
    printf("signal sent to p1 from p2 with PID : %d\n", right);
    fflush(stdout);
    count_right++;
}

int main()
{
    key_t key = ftok("key",65);
    int msqid = msgget(key,IPC_CREAT|0666);
    struct message_buf message;

    struct sigaction act_left={0};
    act_left.sa_flags = SA_SIGINFO;
    act_left.sa_sigaction = &handler_USR1;
    sigaction(SIGUSR1,&act_left,NULL);

    struct sigaction act_right={0};
    act_right.sa_flags = SA_SIGINFO;
    act_right.sa_sigaction = &handler_USR2;
    sigaction(SIGUSR1,&act_right,NULL);

    msgrcv(msqid,&message,sizeof(message.mtext),2,0);
    right = atoi(message.mtext);
    

    for(int i=0;i<3;i++) {
        kill(right,SIGUSR1);
        sleep(3);
    }

    while(count_left<=2)
    {

    }
    sleep(3);
    printf("\nReverse Circular signalling\n");
    fflush(stdout);

    for(int i=0; i<3;i++)
    {
        kill(left,SIGUSR2);
        sleep(3);
    }

    while(count_right<=2)
    {

    }

}