#include"dragon.h"
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include <assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <fcntl.h>
#include<string.h>
#include<time.h>

using namespace std;

#define ERR_EXIT(m) \
    do { \
    perror(m); \
    exit(EXIT_FAILURE); \
    } while (0)
 

Dragon::Dragon(int port,pthread_rwlock_t* rwlock,glove_state* p_state):port(port),rwlock(rwlock),p_state(p_state)
{
}

Dragon::~Dragon()
{
}

int Dragon::init_net()
{
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //fcntl(sock,F_SETFL,O_NONBLOCK);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    printf("listening to port:%d\n",port);
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");
    return 0;
}

int Dragon::hover()
{
    const int SEND_BUF_SIZE=1024;
    char recvbuf[1024] = {0};
    char sendbuf[SEND_BUF_SIZE] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;
    
    while (!should_landing)
    {
        
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        try
        {
            n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        }
        catch(const std::exception& e)
        {
            std::cout <<"timeout!!!"<<std::endl;
            std::cerr << e.what() << '\n';
        }
        
        
        if (n <= 0){
            if (errno == EINTR)
                continue;
            cout <<"recv time out"<<endl;
//            ERR_EXIT("recv time out close net");
        }
        else if(n > 0){
            int total_len;
            switch (recvbuf[0])
            {
            case PT_CHECK_CONNECT:
                sendbuf[0] = PT_CHECK_CONNECT_ACK;
                sendto(sock, sendbuf, 1, 0,(struct sockaddr *)&peeraddr, peerlen);
                break;
            case PT_GRAB_GLOVE_STATE:
                glance_glove_state();
                total_len = this->glove_state_reg.to_bytes(sendbuf,SEND_BUF_SIZE);
                //memcpy(sendbuf,(char*)(this->p_state->angles),sizeof(char)*4*3);
                //sprintf(sendbuf,"sendbuf test");
                sendto(sock, sendbuf, total_len, 0,(struct sockaddr *)&peeraddr, peerlen);
                break;
            default:
                break;
            }
            // printf("received��%s\n",recvbuf);
            // sendto(sock, recvbuf, n, 0,
            //        (struct sockaddr *)&peeraddr, peerlen);
            // printf("send��%s\n",recvbuf);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
    }
    close(sock);
    cout <<"[DRAGON]end hover"<<endl;
    return 0;
}

void Dragon::Run()
{
    this->should_landing = false;
    this->hover();
}

void Dragon::glance_glove_state()
{
    pthread_rwlock_rdlock(rwlock);
    glove_state_reg = (*p_state);
    //cout <<"[Dragon] glance result:" << glove_state_reg.angle[0]<<endl;
    pthread_rwlock_unlock(rwlock);
}

void Dragon::landing()
{
    this->should_landing = true;
}