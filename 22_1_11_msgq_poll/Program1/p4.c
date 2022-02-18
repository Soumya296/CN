#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>

struct msg_buf {

    long mtype;
    char mtext[100];
};

char * message_queue = "./queue";

int main()
{
    key_t key = ftok(message_queue,0);
    int msqid = msgget(key, IPC_CREAT|0666);

    struct msg_buf message;

    message.mtype = 4;
    sprintf(message.mtext, "%d",getpid());
    msgsnd(msqid,&message,sizeof(message.mtext),0);

    msgrcv(msqid,&message,sizeof(message.mtext),4,0);
    printf("%s\n", message.mtext);
    fflush(stdout);

    msgctl(msqid,IPC_RMID, NULL);
}