#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE     10
#define MAX_EVENT_NUM 1024

/* 将描述符设置为非阻塞 */
int setNonBlocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}
/* 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中
 * 参数 enableET 指定是否对 fd 启用 et 模式*/
void addfd(int epollfd, int fd, bool enableET)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enableET)
    {
        event.events |= EPOLLET;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

/* LT 模式工作流程 */
void LT(epoll_event* events, int num, int epollfd, int listenfd)
{
    char buf[BUFF_SIZE] = {0};
    memset(buf, 0x00, BUFF_SIZE);
    for(int i = 0; i < num; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            sockaddr_in clientSockAddr;
            socklen_t   clientSockAddrLen = sizeof(sockaddr_in);
            int clientSock = accept(listenfd, (sockaddr*)&clientSockAddr,
                                    &clientSockAddrLen);
            addfd(epollfd, clientSock, false);
        }
        else if(events[i].events & EPOLLIN)
        {
            std::cout << "event trigger once" << std::endl;
            memset(buf, 0x00, BUFF_SIZE);
            int ret = recv(sockfd, buf, BUFF_SIZE-1, 0);
            if(ret < 0)
            {
                close(sockfd);
                continue;
            }
            buf[ret] = '\0';
            std::cout << "get " << ret << "bytes of connection: [" << buf << "]" << std::endl;
        }
        else
        {
            std::cout << "something else happened " << std::endl;
        }
    }
}

/* ET模式工作流程 */
void ET(epoll_event* events, int num, int epollfd, int listenfd)
{
    char buf[BUFF_SIZE] = {0};
    memset(buf, 0x00, BUFF_SIZE);
    for(int i = 0; i < num; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            sockaddr_in clientSockAddr;
            socklen_t   clientSockAddrLen = sizeof(sockaddr_in);
            int clientSock = accept(listenfd, (sockaddr*)&clientSockAddr, &clientSockAddrLen);
            addfd(epollfd, clientSock, true);
        }
        else if(events[i].events & EPOLLIN)
        {
            std::cout << "event trigger once " << std::endl;
            while(1)
            {
                memset(buf, 0x00, BUFF_SIZE);
                int ret = recv(sockfd, buf, BUFF_SIZE-1, 0);
                if(ret < 0)
                {
                    if((errno == EWOULDBLOCK) || (errno == EAGAIN))
                    {
                        std::cout << "read later" << std::endl;
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0)
                {
                    close(sockfd);
                }
                else
                {
                    buf[ret] = '\0';
                    std::cout << "get " << ret << "bytes of connection: [" << buf << "]" << std::endl;
                }
            }
        }
        else 
        {
            std::cout << "something else happended" << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " ip + port" << std::endl;
        return 1;
    }

    int listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listenSock < 0)
    {
        std::cerr << "socket error" << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));
    socklen_t   serverAddrLen = sizeof(sockaddr_in);
    int ret = bind(listenSock, (sockaddr*)&serverAddr, serverAddrLen);
    if(ret < 0)
    {
        std::cerr << "bind error" << std::endl;
        return -1;
    }

    ret = listen(listenSock, 5);
    if(ret < 0)
    {
        std::cerr << "listen error" << std::endl;
        close(listenSock);
        return -1;
    }
    epoll_event events[MAX_EVENT_NUM];
    int epollfd = epoll_create(5);
    if(epollfd < 0)
    {
        std::cerr << "epoll_create error" << std::endl;
        close(listenSock);
        return -1;
    }
    addfd(epollfd, listenSock, true);
    while(1)
    {
        int ret_wait = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1);
        if(ret < 0)
        {
            std::cerr << "epoll_wait error" << std::endl;
            close(listenSock);
            return -1;
        }

        LT(events, ret_wait, epollfd, listenSock);
        // ET(events, ret_wait, epollfd, listenfd);
    }

    close(listenSock);
    return 0;
}
