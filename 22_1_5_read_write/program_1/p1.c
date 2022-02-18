#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

int read_next_num(int fd, int *eof)
{
	int num = 0;
	int sz;

	bool numread = false;
	char ch;

	sz = read(fd, &ch, 1);
	if (sz == 0)
	{
		*eof = 0;
		return -1;
	}

	while (sz != 0)
	{
		if (!numread)
		{
			if (ch == ' ' || ch == '\n' || ch == '\t')
				continue;
		}
		else
		{
			if (ch == ' ' || ch == '\n' || ch == '\t')
				break;
		}

		numread = true;

		num = num * 10 + (ch - '0');
		sz = read(fd, &ch, 1);
		if (sz == 0)
			*eof = 0;
	}
	return num;
}

char *toArray(int number)
{
	int n = log10(number) + 1;
	int i;
	char *numberArray = calloc(n, sizeof(char));
	for (i = n - 1; i >= 0; --i, number /= 10)
	{
		numberArray[i] = (number % 10) + '0';
	}
	return numberArray;
}

int main()
{
	char *buf;
	int fd1 = open("f1.txt", O_RDONLY);
	int fd2 = open("f2.txt", O_RDONLY);
	int fd3 = open("f3.txt", O_CREAT | O_TRUNC | O_APPEND | O_WRONLY, 0777);

	int num1, num2;
	int eof1 = 1;
	int eof2 = 1;

	num1 = read_next_num(fd1, &eof1);
	num2 = read_next_num(fd2, &eof2);

	while (eof1 && eof2 && num1 != -1 && num2 != -1)
	{
		if (num1 < num2)
		{
			buf = toArray(num1);
			write(fd3, buf, sizeof(buf) / sizeof(char));
			write(fd3, " ", 1);
			num1 = read_next_num(fd1, &eof1);
		}
		else
		{
			buf = toArray(num2);
			write(fd3, buf, sizeof(buf) / sizeof(char));
			write(fd3, " ", 1);
			num2 = read_next_num(fd2, &eof2);
		}
	}

	if (!eof1)
	{
		while (eof2)
		{
			buf = toArray(num2);
			write(fd3, buf, sizeof(buf) / sizeof(char));
			write(fd3, " ", 1);
			num2 = read_next_num(fd2, &eof2);
		}
	}

	else if (!eof2)
	{
		while (eof1)
		{
			buf = toArray(num1);
			write(fd3, buf, sizeof(buf) / sizeof(char));
			write(fd3, " ", 1);
			num1 = read_next_num(fd1, &eof1);
		}
	}

	write(fd3, "\n", 1);
	close(fd1);
	close(fd2);
	close(fd3);

	return 0;
}
