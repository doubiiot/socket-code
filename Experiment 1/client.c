#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8887	/*侦听端口地址*/
#define BUFFER_SIZE 1024
int main(int argc, char *argv[])
{
	int s;		//s为socket描述符
	struct sockaddr_in server_addr;	//服务器地址结构

	char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

	s = socket(AF_INET, SOCK_STREAM, 0); //建立一个流式套接字
	if(s < 0)
    {
		perror("socket error");
		return -1;
	}
    //设置服务器地址
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;//协议族
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//本地地址
	server_addr.sin_port = htons(PORT);

	//连接服务器
	if((connect(s, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))) == -1)
    {
        perror("connect error");
        return -1;
    }
    printf("connect success!\n");
	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        printf("send to server:%s\n",sendbuf);
        send(s, sendbuf, strlen(sendbuf),0); //发送
        if(strcmp(sendbuf,"exit\n")==0)
            break;
        recv(s, recvbuf, sizeof(recvbuf),0); //接收
        printf("recv from server:%s\n",recvbuf);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
	close(s);
	return 0;
}


