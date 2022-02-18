#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <bits/stdc++.h>
#include <signal.h>

using namespace std;

struct record{
    pid_t pid;
    char name;
    int sf1_count;
    int sf2_count;
};

map<pid_t, struct record> directory;

void handler_USR1(int sifid, siginfo_t *info, void *context)
{
    pid_t temp = info->si_pid;
    auto it = directory.find(temp);
    if( it != directory.end())
    {
        it->second.sf1_count++;
    }
    else{
        struct record temp_process;
        temp_process.pid = temp;
        temp_process.name = directory.size()+65;
        temp_process.sf1_count = 1;
        temp_process.sf2_count = 0;
        directory.insert(pair<pid_t, struct record>(temp_process.pid,temp_process));
    }
}
void handler_USR2(int sifid, siginfo_t *info, void *context)
{
    pid_t temp = info->si_pid;
    auto it = directory.find(temp);
    if( it != directory.end())
    {
        it->second.sf2_count++;
    }
    else{
        struct record temp_process;
        temp_process.pid = temp;
        temp_process.name = directory.size()+65;
        temp_process.sf2_count = 1;
        temp_process.sf1_count = 0;
        directory.insert(pair<pid_t, struct record>(temp_process.pid,temp_process));
    }
}

int main()
{
    struct sigaction act_left={0};
    act_left.sa_flags = SA_SIGINFO;
    act_left.sa_sigaction = &handler_USR1;
    sigaction(SIGUSR1,&act_left,NULL);

    struct sigaction act_right={0};
    act_right.sa_flags = SA_SIGINFO;
    act_right.sa_sigaction = &handler_USR2;
    sigaction(SIGUSR2,&act_right,NULL);

    while(1)
    {
        sleep(5);
        cout<<"Process\t";

        for(auto i: directory)
        {
            cout<<i.second.name<<"\t";
        }
        cout<<endl<<"Sf1\t";
        for(auto i: directory)
        {
            cout<<i.second.sf1_count<<"\t";
        }
        cout<<endl<<"Sf2\t";
        for(auto i: directory)
        {
            cout<<i.second.sf2_count<<"\t";
        }
        cout<<endl;
    }
}

