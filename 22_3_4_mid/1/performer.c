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
int MSG_QUEUE_START = 0;

void handler_USR1(int sifid, siginfo_t *info, void *context)
{
    MSG_QUEUE_START = 1;
}


int main()
{
    /*HAndling SIGUSR1*/
    struct sigaction act={0};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handler_USR1;
    sigaction(SIGUSR1,&act,NULL);

    /*Init of Message Queue*/
    key_t key = ftok("./queue",0);
    int msqid = msgget(key, IPC_CREAT|0666);
    struct msg_buf message;

    /*Socket creation adn connection to Screen*/
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd == 0) perror("Socket()\n");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(sfd,(struct sockaddr *)&address,sizeof(address))<0)
    {
        printf("connection failed\n");
        return 0;
    }
    else printf("\nConnection to Screen Successful\n");

    printf("Sending Performance to the Screen\n");

    /*Sending Performance to the Screen*/
    char buf_perf[] = "random_performance\n";
    for(int i=0; i<3; i++)
    {
        send(sfd, buf_perf,sizeof(buf_perf),0);
    }

    sprintf(buf_perf, "complete");
    printf("Closing Performance\n");
    send(sfd,buf_perf,sizeof(buf_perf),0);


    while(!MSG_QUEUE_START){}; /*Wait for the signal from anchor to start reading the message from msg queue*/

    /*Getting the pid of anchor to get the judges' Process ID*/
    int fd = fileno(popen("pidof ./anchor", "r"));
    char s[1024];
    read(fd,&s,1024);
    close(fd);
    pid_t pid_anchor = atoi(s);

    /*Read the PID of the chosen judge*/
    msgrcv(msqid,&message,sizeof(message.mtext),pid_anchor,0);
    pid_t judge = atoi(message.mtext);

    /*Communicate with the ANchor and send reply to the screen*/
    while(1)
    {
        msgrcv(msqid, &message,sizeof(message.mtext),judge, 0);
        printf("Question from the Judge : %s \nSending reply to screen\n", message.mtext);
        if(strncmp(message.mtext,"done",4)== 0) break;
        sprintf(buf_perf, "random_reply\n");
        send(sfd,buf_perf,sizeof(buf_perf),0);
    }

    sprintf(buf_perf, "complete");
    send(sfd,buf_perf,sizeof(buf_perf),0);


    printf("Q&A done\nWaiting for Scores\n");
    close(sfd);
    return 0;
}