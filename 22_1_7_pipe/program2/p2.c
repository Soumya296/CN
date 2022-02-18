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

    // int psz, csz;
    char *buf = NULL;

    int std_in = dup(0);
    int std_out = dup(1);

    if (p > 0)
    {
        close(pfd[0]);
        while (1)
        {
            dup2(std_in, 0);
            dup2(std_out, 1);

            printf("Enter the input\n");
            scanf("%s", buf);

            if (!buf)
            {
                printf("Null input..terminating!!\n");
                break;
            }

            dup2(pfd[1], 1);
            printf("%s", buf);
            dup2(cfd[0], 0);
            scanf("%s", buf);

            dup2(std_in, 0);
            dup2(std_out, 1);
        }
        close(pfd[1]);
    }
    else if (p == 0)
    {
        close(cfd[0]);
        while (1)
        {
            dup2(std_in, 0);
            dup2(std_out, 1);

            printf("In process 2\n");
            execv();
        }
        close(cfd[1]);
    }
}