#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>
#include<signal.h>

struct msg_buf {
    long mtype;
    char mtext[100];
};

struct neighbor {
    char left[10];
    char right[10];
    pid_t left_pid;
    pid_t right_pid;
};

struct neighbor nh;
char * message_queue = "./queue";

int count_left=0;
int count_right=1;


void handler_usr1(int sifid, siginfo_t *info, void *context)
{
    count_left++;
    sprintf(nh.left,"P4");
    nh.left_pid = info->si_pid;
    printf("Right : Signal sent from process with pid %d\n",info->si_pid);
    // if(count_left<=3) kill(nh.right_pid,SIGUSR1);
    fflush(stdout);
    
}

void handler_usr2_left(int sifid, siginfo_t *info, void *context)
{
    count_right++;
    printf("left : Signal sent from process with pid %d\n",info->si_pid);
    // if(count_right<=3) kill(nh.left_pid,SIGUSR2);
    fflush(stdout);
    
}

int main()
{
    key_t key = ftok(message_queue,0);
    int msqid = msgget(key,IPC_CREAT|0666);

    struct msg_buf message;


    msgrcv(msqid,&message,sizeof(message.mtext),2,0);
    sprintf(nh.right,"P2");
    nh.right_pid = atoi(message.mtext);

    kill(nh.right_pid, SIGUSR1);

    struct sigaction action1 = {0};
    action1.sa_flags = SA_SIGINFO;
    action1.sa_sigaction = &handler_usr1;
    sigaction(SIGUSR1,&action1,NULL);

    for(int i=0; i<3; i++)
    {
        kill(nh.right_pid,SIGUSR1);
        sleep(5);
    }

    while(count_left<3)
    {

    }

    struct sigaction action_left = {0};
    action1.sa_flags = SA_SIGINFO;
    action_left.sa_sigaction= &handler_usr2_left;
    sigaction(SIGUSR2,&action_left,NULL);

    for(int i=0; i<3; i++)
    {
        kill(nh.left_pid,SIGUSR2);
        sleep(5);
    }

}