#include<unistd.h>
#include<fcntl.h>

int main()
{
	char buf[10];
	//int buf[10] = {1,2,3,4,5,13,15,21,25,22};
	int fd = open("file1.txt", O_CREAT|O_RDONLY, 666);
	int sz = read(fd, buf,10);
	int sz_w = write(1,(sz+'0'),4);
	return 0;
}
