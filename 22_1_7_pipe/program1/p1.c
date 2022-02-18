#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
    int pfd[2], cfd[2];

    if (pipe(pfd) == -1)
    {
        printf("Error while creating parent pipe\n");
        return 1;
    }
    if (pipe(cfd) == -1)
    {
        printf("Error while creating child pipe\n");
        return 1;
    }

    pid_t p = fork();

    if (p < 0)
        printf("Error while creating child process\n");
    int psz, csz;
    char buf[100];

    if (p > 0)
    {
        close(pfd[0]);
        while (1)
        {
            write(1, "Parent Process in Execution\n", strlen("Parent Process in Execution\n"));
            write(1, "Enter the input through keyboard\n", strlen("Enter the input through keyboard\n"));
            psz = read(0, buf, 100);
            if (psz == 1 && buf[0] == '\n')
            {
                write(1, "Null input... terminating!!\n", strlen("Null input... terminating!!\n"));
                break;
            }
            write(pfd[1], buf, psz);
            write(1, "Written to parent pipe\n", strlen("Written to parent pipe\n"));
            csz = read(cfd[0], buf, 100);
            write(1, "Read from child pipe\n", strlen("Read from child pipe\n"));
            write(1, buf, csz);
        }
        close(pfd[1]);
    }
    else if (p == 0)
    {
        close(cfd[0]);
        while (1)
        {
            write(1, "Child Process in Execution\n", strlen("Child Process in Execution\n"));
            psz = read(pfd[0], buf, 100);
            write(1, "Read from parent pipe\n", strlen("Read from parent pipe\n"));
            write(1, buf, psz);
            write(1, "Enter the input through keyboard\n", strlen("Enter the input through keyboard\n"));
            csz = read(0, buf, 100);
            if (psz == 1 && buf[0] == '\n')
            {
                write(1, "Null input... terminating!!\n", strlen("Null input... terminating!!\n"));
                break;
            }
            write(cfd[1], buf, csz);
            write(1, "Written to child pipe\n", strlen("Written to child pipe\n"));
        }
        close(cfd[1]);
    }

    return 0;
}