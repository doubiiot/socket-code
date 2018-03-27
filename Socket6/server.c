#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/resource.h>

#define QLEN       32
#define BUFSIZE    1024
#define UDP_SERVER 0
#define TCP_SERVER 1
#define NOSOCK     -1
#define LENGTH     72
#define UNIXEPOCH  2208988800UL

extern int errno;
unsigned short portbase = 0;

#ifndef MAX
#define MAX(x,y)   ((x)>(y) ? (x) : (y))
#endif

void ServerEcho(int fd);
void ServerChargen(int fd);
void ServerDaytime(int fd);
void ServerTime(int fd);

struct service
{
    char *sv_name;
    char sv_useTcp;
    int sv_sock;
    void (*sv_func)(int);
};

struct service svent[]=
{
    {"echo",TCP_SERVER,NOSOCK,ServerEcho},
    {"chargen",TCP_SERVER,NOSOCK,ServerChargen},
    {"daytime",TCP_SERVER,NOSOCK,ServerDaytime},
    {"time",UDP_SERVER,NOSOCK,ServerTime},
     {0,0,0,0},
};

int passivesock(const char *service, const char *transport, int qlen)
{
    struct servent *pse;
    struct protoent *ppe;
    struct sockaddr_in sin;
    int s,type;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    if( pse = getservbyname(service, transport) )
    {
        sin.sin_port = htons(ntohs((unsigned short)pse->s_port)+portbase);
    }
    else if( (sin.sin_port = htons((unsigned short)atoi(service))) == 0 )
    {
        printf("can not get \" %s \" service entry . \n", service);
        return -1;
    }

    if( (ppe = getprotobyname(transport)) == 0)
    {
        printf("can not get \" %s \" protocol entry .\n", transport);
    }
    if( strcmp(transport, "udp") == 0)
    {
        type = SOCK_DGRAM;
    }
    else
    {
        type = SOCK_STREAM;
    }

    s = socket(PF_INET, type, ppe->p_proto);
    if(s < 0)
    {
        printf("can not create socket : %s \n", strerror(errno));
        return -1;
    }

    if(bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("can not bind to %s port : %s . \n", service, strerror(errno));
    }
    if(type == SOCK_STREAM && listen(s,qlen) < 0)
    {
        printf("can not listen on %s port : %s \n", service, strerror(errno));
    }
    return s;
}

int passiveTCP(const char *service, int qlen)
{
    return passivesock(service,"tcp",qlen);
}

int passiveUDP(const char *service)
{
    return passivesock(service,"udp",0);
}

void doTcp(struct service *psv)
{
    struct sockaddr_in fsin;
    unsigned int alen;
    int fd,ssock;
    alen = sizeof(fsin);
    ssock = accept(psv->sv_sock, (struct sockaddr *)&fsin, &alen);
    if(ssock < 0)
    {
        printf("accept : %s . \n", strerror(errno));
        return;
    }
    switch(fork())
    {
    case 0:
        break;
    case -1:
        printf("fork: %s . \n",strerror(errno));
        return;
    default:
        close(ssock);
        return;
    }
    for(fd=NOFILE; fd>=0 ; --fd)
    {
        if(fd != ssock)
        {
            close(fd);
        }
    }
    psv->sv_func(ssock);
    return;
}

void reaper(int sig)
{
    int status;
    while(wait3(&status,WNOHANG,(struct rusage *)0) >= 0);
}

void ServerEcho(int fd)
{
    char buf[BUFSIZE];
    int cc;
    while(cc = read(fd, buf, sizeof(buf)))
    {
        if(cc < 0)
        {
            printf("echo read : %s .\n",strerror(errno));
            return;
        }
        if(write(fd, buf, cc) < 0)
        {
            printf("echo write : %s .\n",strerror(errno));
            return;
        }
    }
}

void ServerChargen(int fd)
{
    char c;
    char buf[LENGTH+2];
    c = ' ';
    buf[LENGTH] = '\r';
    buf[LENGTH] = '\n';
    while(1)
    {
        int i;
        for(i = 0; i < LENGTH; ++i)
        {
            buf[i] = c++;
            if(c > '~')
            {
                c=' ';
            }
        }
        if(write(fd,buf,LENGTH+2) < 0)
        {
            break;
        }
    }
}

void ServerDaytime(int fd)
{
    char buf[LENGTH];
    char *ctime();
    time_t now;
    time(&now);
    sprintf(buf,"%s",ctime(&now));
    write(fd,buf,strlen(buf));
}

void ServerTime(int fd)
{
    time_t now;
    struct sockaddr_in fsin;
    char buf[512];
    unsigned int alen;
    time(&now);
    now = htonl((unsigned long)(now+UNIXEPOCH));
    alen = sizeof(fsin);
    if(recvfrom(fd,buf,sizeof(buf),0,(struct sockaddr *)&fsin,&alen) < 0)
     {
        printf("receive data fail , reason is %s .\n",strerror(errno));
        return;
     }
    else
     {
           printf("receive data succes . \n");
          if(sendto(fd,(char *)&now,sizeof(now),0,(struct sockaddr *)&fsin,sizeof(fsin)) < 0)
             {
                  printf("send data fail. \n");
             }
          else
             {
                  printf("send data success . \n");
             }
     }
}

int main(int argc, char *argv[])
{
    struct service *psv;
    struct service *fd2sv[NOFILE];//最大文件打开数
    fd_set rfds,afds;
    int fd,nfds;
    //设置端口
    switch(argc)
    {
    case 1:
        break;
    case 2:
        portbase = (unsigned short) atoi(argv[1]);
        break;
    default:
        printf("port is busy !");
        break;
    }

    nfds = 0;
    FD_ZERO(&afds);
    for(psv = &svent[0]; psv->sv_name; ++psv)
    {
        if(psv->sv_useTcp)
        {
            psv->sv_sock = passiveTCP(psv->sv_name,QLEN);
        }
        else
        {
            psv->sv_sock = passiveUDP(psv->sv_name);
        }
        fd2sv[psv->sv_sock] = psv;
        nfds = MAX(psv->sv_sock+1,nfds);
        FD_SET(psv->sv_sock,&afds);
    }
    signal(SIGCHLD, reaper);

    while(1)
    {
        memcpy(&rfds, &afds, sizeof(rfds));
        if(select(nfds,&rfds,(fd_set *)0,(fd_set *)0,(struct timeval *)0)<0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            printf("select: %s .\n",strerror(errno));
            return -1;
        }
        for(fd = 0; fd < nfds; ++fd)
        {
            if(FD_ISSET(fd, &rfds))
            {
                psv = fd2sv[fd];
                if(psv->sv_useTcp)
                {
                    doTcp(psv);
                }
                else
                {
                    psv->sv_func(psv->sv_sock);
                }
            }
        }
    }

    return 0;
}
