//poll
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <unistd.h>
#include <netinet/in.h>

#include <errno.h>

#define MAX_LISTEN_NUM 10
#define BUFFER_SIZE 1024
#define POLL_SIZE 1024

int main(int argc,char* argv[]){

    if(argc<2){
        printf("need port!");
        return -1;
    }

    int sockfd=socket(PF_INET,SOCK_STREAM,0);

    struct sockaddr_in addr={0};
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(atoi(argv[1]));

    int ret=bind(sockfd,(struct sockaddr*)&addr,sizeof(struct sockaddr));
    if(ret == -1){
        perror("bind");
        return -2;
    }

    ret=listen(sockfd,MAX_LISTEN_NUM);
    if(ret == -1){
        perror("listen");
        return -3;
    }

    struct sockaddr client_addr={0};
    int client_len=sizeof(struct sockaddr);
    int clientfd=0;

    struct pollfd fds[POLL_SIZE]={0};
    fds[sockfd].fd=sockfd;
    fds[sockfd].events=POLLIN;
    int maxfd=sockfd;

    while(1){

        int nready=poll(fds,maxfd+1,-1);

        if(nready==-1)
        {
            perror("poll");
            return -4;
        }

        if(fds[sockfd].revents & POLLIN){
            clientfd=accept(sockfd,(struct sockaddr*)&client_addr,&client_len);
            if(clientfd==-1)
            {
                perror("accpet");
                return -5;
            }

            fds[clientfd].fd=clientfd;
            fds[clientfd].events=POLLIN;

            if(clientfd>maxfd)maxfd=clientfd;
            if(nready==1)continue;
        }

        for(int i=sockfd+1;i<=maxfd;i++){
            if(fds[i].revents & POLLIN){
                char buffer[BUFFER_SIZE]={0};
                int len=recv(i,buffer,BUFFER_SIZE,0);
                if(len==0){
                    fds[i].events=-1;
                    fds[i].fd=-1;
                    close(i);
                    break;
                }
                printf("clientfd:%d len:%d buffer:%s\n",clientfd,len,buffer);

            }

        }
    }

    close(sockfd);
    return 0;
}