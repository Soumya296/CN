#include<bits/stdc++.h>
#include<fcntl.h>
#include<unistd.h>
using namespace std;

int main()
{
    // int fd = open("IT.txt",O_CREAT);
    // dup2(fd,0);
    int a,b;
    cin>>a>>b;
    cout<<a+b<<" ";
    cin>>a>>b;
    cout<<a-b<<" ";
    cin>>a>>b;
    cout<<a*b;

    return 0;
}