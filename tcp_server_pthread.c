//使用多线程实现
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include <errno.h>

#define MAX_LISTEN_NUM 10   //最大同时listen数量
#define BUFFER_SIZE 1024    //读取缓冲大小

void *Client_Call_Back(void* arg){
    int clientfd=*(int *)arg;
    //取得客户端句柄

    while(1)
    {
        char buffer[1024]={0};
        int len=recv(clientfd,buffer,BUFFER_SIZE,0);
        if(len==0){
            close(clientfd);
            break;
        }
        printf("clientfd=%d len=%d buffer=%s\n",clientfd,len,buffer);
    }

    return (void*)0;
}

int main(int argc,char* argv[]){

    if(argc<2)
    {
        printf("need port!\n");
        return -1;
    }

    int sockfd=socket(PF_INET,SOCK_STREAM,0);  //创建套接字 用来listen端口连接请求

    struct sockaddr_in addr;//给sockfd设置地址
    memset(&addr,0,sizeof(struct sockaddr_in));

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);//0.0.0.0 允许接受任意ip的连接请求
    addr.sin_port=htons(atoi(argv[1]));//设置输入的端口号

    int ret=bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr));//将sockkfd设置好地址
    if(ret==-1){
        perror("bind");
        return -2;
    }

    ret=listen(sockfd,MAX_LISTEN_NUM);
    if(ret==-1){
        perror("listen");
        return -3;
    }
    //sockfd设置完毕

    //初始化客户端地址和句柄
    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    int client_len=sizeof(client_addr);
    int clientfd=0;

    //服务器开始运行
    while(1){
        clientfd=accept(sockfd,(struct sockaddr*)&client_addr,&client_len);//接受新的连接申请
        if(clientfd == -1)
        {
            perror("accept");
            return -4;
        }
 
        pthread_t threadid;//为新连接创建新线程
        pthread_create(&threadid,NULL,Client_Call_Back,&clientfd);
    }

    close(sockfd);
    return 0;
}