#include<unistd.h>
#include<fcntl.h>

int main()
{
	char buf[1];
	int fd1 = open("f1.txt", O_CREAT|O_RDONLY, 0777);
	int fd2 = open("f2.txt", O_CREAT|O_TRUNC|O_WRONLY|O_APPEND, 0777);

	int sz = read(fd1, buf, 1);
	while(sz != 0)
	{
		if((int)buf[0] >96 && (int)buf[0] <123)
		{
			int temp = (int)buf[0] - (int)32;
			buf[0] = (char)temp;
		}
			
		write(fd2, buf, sz);
		sz = read(fd1, buf, 1);
	}

	return 0;
}
