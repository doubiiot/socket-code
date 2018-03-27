#pragma comment(lib,"wsock32.lib")
#include <winsock2.h>
#include <stdio.h>



#define MaxLen 409600
#define PORT 8887
typedef struct sockaddr_in addr;

int InitSockt(void)
{
    WORD sockVersion = MAKEWORD(2,2);

    WSADATA data;
    if(WSAStartup(sockVersion, &data) != 0)
    {
        printf("winsock not avaliable!\n");
        return 0;
    }
    printf("init socket success!\n");
    return 1;

}
int main(int argc, char* argv[])
{

    SOCKET client;
    int err;
    char bufferw[MaxLen];
    char bufferr[MaxLen];
    addr server_add;
    int i;

    InitSockt();

    if((client = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("invalid socket!\n");
        return 1;
    }

    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(PORT);
    server_add.sin_addr.S_un.S_addr = inet_addr("192.168.101.151");

    if (connect(client, (struct sockaddr*)&server_add, sizeof(addr)) == INVALID_SOCKET)
    {
        printf("connect error!\n");
        return 1;
    }
    else
    {
        printf("connect success!\n");
    }
    char file_name[MaxLen + 1];
    memset(file_name,0,MaxLen);
    printf("Please Input File Name On Server:\n");
    scanf("%s", file_name);
    FILE *fp = fopen(file_name,"wb");
    if(fp == NULL)
    {
        printf("file %s can not open to write!\n",file_name);
        return 1;
    }
    if(send(client,file_name,sizeof(file_name),0) == SOCKET_ERROR)
    {
        printf("send error!\n");
        return 1;
    }
    int length = 0;
    while((length = recv(client,bufferw,sizeof(bufferw),0)) > 0)
    {

        int write_length = fwrite(bufferw,sizeof(char),length,fp);
        if(write_length < length)
        {
            printf("write failed!\n");
            break;
        }
        memset(bufferw,0,MaxLen);

    }

    printf("recv file finished!\n");
    fclose(fp);
    closesocket(client);
    WSACleanup();
    return 0;
}
