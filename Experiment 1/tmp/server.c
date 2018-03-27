#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8887
#define BACKLOG 2	//侦听队列长度
#define FILE_MAX_SIZE 1024
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
	int ss;	                        //ss为服务器的socket，sc为客户端
	ss = socket(AF_INET, SOCK_STREAM, 0);
	if(ss < 0)
	{
		printf("socket error\n");
		exit(1);
	}

    struct sockaddr_in server_addr;	    //服务器地址结构
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr =  htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

    //绑定地址结构到套接字描述符
	if(bind(ss, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("bind error");
		exit(1);
	}
	//设置侦听
	if(listen(ss, BACKLOG) < 0){
		perror("listen error");
		exit(1);
	}
    printf("server start!\n");
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int sc = accept(ss, (struct sockaddr*)&client_addr, &addrlen);
    if(sc < 0)
    {
        perror("accept");
    }
	while(1)
	{


        char buffer[BUFFER_SIZE];
        bzero(buffer,sizeof(buffer));

        if(recv(sc, buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("recv data ");
            exit(1);
        }
        printf("recv from client:%s\n",buffer);
        if (send(sc, buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("Send:");
            exit(1);
        }
	}
}
