//使用select实现
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>

#include <errno.h>

#define MAX_LISTEN_NUM 10
#define BUFFER_SIZE 1024

int main(int argc,char* argv[]){
    if(argc<2){
        printf("need port!\n");
        return -1;
    }

    int sockfd=socket(PF_INET,SOCK_STREAM,0);
    
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(atoi(argv[1]));

    int res=bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr));
    if(res==-1){
        perror("bind");
        return -2;
    }

    res=listen(sockfd,MAX_LISTEN_NUM);
    if(res==-1){
        perror("listen");
        return -3;
    }

    struct sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(struct sockaddr));
    int client_len=sizeof(client_addr);
    int clientfd=0;

    fd_set rfds,rfds_copy;
    FD_ZERO(&rfds);
    FD_SET(sockfd,&rfds);
    int maxfd=sockfd;

    while(1){
        rfds_copy=rfds;
        int nready=select(maxfd+1,&rfds_copy,NULL,NULL,NULL);

        if(FD_ISSET(sockfd,&rfds_copy)){
            clientfd=accept(sockfd,(struct sockaddr*)&client_addr,&client_len);
            if(clientfd==-1){
                perror("accept");
                return -4;
            }

            FD_SET(clientfd,&rfds);
            if(clientfd>maxfd)maxfd=clientfd;
            if(nready==1)continue;
        }

        for(int i=sockfd+1;i<=maxfd;i++){
            if(FD_ISSET(i,&rfds_copy)){
                char buffer[BUFFER_SIZE]={0};
                int len=recv(i,buffer,BUFFER_SIZE,0);
                if(len==0){
                    FD_CLR(i,&rfds);
                    close(i);
                    break;
                }
                printf("clientfd:%d len:%d buffer:%s\n",i,len,buffer);
            }
        }

    }
    
    close(sockfd);
    return 0;
}
