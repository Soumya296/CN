#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
    int fd = fileno(popen("./p3", "w"));
    dup2(fd, 1);
    while (1)
    {
        char buf[1024];
        scanf("%s", buf);
        printf("%s ", buf);
        fflush(stdout);
    }
}