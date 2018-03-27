#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 8887
#define BACKLOG 2	//侦听队列长度

int main(int argc, char *argv[])
{
	int ss,sc;	//ss为服务器的socket，sc为客户端
	struct sockaddr_in server_addr;	//服务器地址结构
	struct sockaddr_in client_addr;	//客户端地址结构
	pid_t pid;				/*分叉的进行ID*/
	ss = socket(AF_INET, SOCK_STREAM, 0);
	if(ss < 0)
	{
		printf("socket error\n");
		return -1;
	}

	//设置服务器地址
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;	//协议族
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//本地地址
	server_addr.sin_port = htons(PORT);	//服务器端口

    //绑定地址结构到套接字描述符
	if(bind(ss, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("bind error");
		return -1;
	}

	//设置侦听
	if(listen(ss, BACKLOG) < 0){
		perror("listen error\n");
		return -1;
	}
	printf("Server started!\n");
	socklen_t addrlen = sizeof(struct sockaddr);
    sc = accept(ss, (struct sockaddr*)&client_addr, &addrlen);
    if(sc < 0){
        perror("accept:");
    }
    printf("client connected!");
	while(1)
	{
		pid = fork();
		if( pid == 0 )
        {
            /*子进程中*/
            while(1)
            {
                char buffer[1024];
                int len = recv(sc, buffer, sizeof(buffer),0);
                if(strcmp(buffer,"exit\n")==0 || len<=0)
                    break;
                printf("data from client:%s\n",buffer);
                send(sc, buffer, len, 0);
                printf("data send to client:%s\n",buffer);
            }
                close(ss);	/*在子进程中关闭服务器的侦听*/

		}
		else
        {
            close(sc);	/*在父进程中关闭客户端的连接*/
		}
	}
}
