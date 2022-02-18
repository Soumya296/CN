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
    int p2,p3,p4;
    key_t key = ftok(message_queue,0);

    int msqid = msgget(key, IPC_CREAT|0666);

    struct msg_buf message;

    msgrcv(msqid,&message,sizeof(message.mtext),2,0);
    p2 = atoi(message.mtext);
    printf("Message content from p2: PID = %d\n", p2);
    message.mtype = 2;
    sprintf(message.mtext, "Message from P1 to P2");
    msgsnd(msqid,&message,sizeof(message.mtext),0);
    fflush(stdout);

    msgrcv(msqid,&message,sizeof(message.mtext),3,0);
    p3 = atoi(message.mtext);
    printf("Message content from p3: PID = %d\n", p3);
    message.mtype = 3;
    sprintf(message.mtext, "Message from P1 to P3");
    msgsnd(msqid,&message,sizeof(message.mtext),0);
    fflush(stdout);

    msgrcv(msqid,&message,sizeof(message.mtext),4,0);
    p4 = atoi(message.mtext);
    printf("Message content from p4: PID = %d\n", p2);
    message.mtype = 4;
    sprintf(message.mtext, "Message from P1 to P4");
    msgsnd(msqid,&message,sizeof(message.mtext),0);
    fflush(stdout);

    // msgctl(msqid,IPC_RMID, NULL);
}
