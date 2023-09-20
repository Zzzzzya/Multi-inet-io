//使用epoll(水平触发)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>

#define MAX_LISTEN_NUM 10
#define EVENTS_NUM 1024
#define BUFFER_SIZE 1024

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
    if(ret==-1){
        perror("bind");
        return -1;
    }

    ret=listen(sockfd,MAX_LISTEN_NUM);
    if(ret==-1){
        perror("listen");
        return -2;
    }

    struct sockaddr_in client_addr={0};
    int client_len=sizeof(struct sockaddr_in);
    int clientfd=0;

    int epfd=epoll_create(1);
    if(epfd==-1){
        perror("epoll_create failed");
        return -3;
    }

    struct epoll_event sock_listen_ev;
    sock_listen_ev.events=EPOLLIN;
    sock_listen_ev.data.fd=sockfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&sock_listen_ev);

    struct epoll_event events[EVENTS_NUM];

    while(1){
        int nready=epoll_wait(epfd,events,EVENTS_NUM,-1);
        
        if(nready==-1){
            perror("epoll_wait");
            return -4;
        }

        for(int i=0;i<=nready;i++){
            int curfd=events[i].data.fd;
            if(curfd==sockfd){
                clientfd=accept(sockfd,(struct sockaddr*)&client_addr,&client_len);
                if(clientfd==-1){
                    perror("accept");
                    return -5;
                }

                struct epoll_event client_ev;
                client_ev.data.fd=clientfd;
                client_ev.events=EPOLLIN;
                epoll_ctl(epfd,EPOLL_CTL_ADD,clientfd,&client_ev);
            } else if (events[i].events == EPOLLIN){
                char buffer[BUFFER_SIZE]={0};
                int len=recv(curfd,buffer,BUFFER_SIZE,0);
                if(len==0){
                    epoll_ctl(epfd,EPOLL_CTL_DEL,curfd,NULL);
                    close(curfd);
                    break;
                }
                printf("clientfd:%d len=%d buffer=%s\n",curfd,len,buffer);
            }
        }
    }
    close(sockfd);
    return 0;
}