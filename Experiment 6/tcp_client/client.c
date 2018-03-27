#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define UNIXEPOCH 2208988800UL
#define BUFFER_SIZE 2048

extern int errno;

int connectTCP(const char *host, const char *service, const char *transport)
{
    struct hostent * phe;
    struct servent *pse;

    struct sockaddr_in sin;
    int s,type;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    if(pse = getservbyname(service, transport))
    {
        sin.sin_port = pse->s_port;
    }
    else if((sin.sin_port = htons((unsigned short)atoi(service))) == 0)
    {
        printf("can not get \" %s \" service entry . \n", service);
        return -1;
    }

    if(phe = gethostbyname(host))
    {
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    }
    else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
    {
        printf("can not get \" %s \" host entry .", host);
        return -1;
    }

    s=socket(PF_INET, SOCK_STREAM, 0);
    if(s < 0)
    {
        perror("socket");
        exit(1);
    }
    if(connect(s,(struct sockaddr *)&sin, sizeof(sin))<0)
    {
        perror("connect");
        exit(1);
    }
    printf("create sock success!\n");
    return s;
}
void TcpProcess(const char * host, const char * service)
{
  int sock_fd;
  int size = 0;
  char buffer[BUFFER_SIZE];

  sock_fd=connectTCP(host,service,"tcp");

   for(;;)
   {
     size = read(0 , buffer , BUFFER_SIZE);
     if(size > 0)
     {
       write(sock_fd, buffer, size);
       size = read(sock_fd, buffer , BUFFER_SIZE);
       if(size > 0)
         {
           buffer[size] = '\0';
           if((strcmp(service,"time")==0)||(atoi(service) == 37))
              {
                 time_t now = ntohl((unsigned long)strtoul(buffer,NULL,10)-UNIXEPOCH);
                 printf("current time is : %s",ctime(&now));
              }
           else
             {
                printf("receive data is : %s", buffer);
             }
         }
       else
         {
          printf("read data fail .\n");
          return ;
         }
     }
   }

}

int main(int argc, char *argv[])//
{
    char *host="localhost";
    char *service="daytime";
    switch(argc)
    {
    case 1:
        host = "localhost";
        break;
    case 3:
        service = argv[2];
    case 2:
        host = argv[1];
        break;
    default:
        printf("port or host is not valid . \n");
        return -1;
    }
    TcpProcess(host, service);
    return 0;
}
