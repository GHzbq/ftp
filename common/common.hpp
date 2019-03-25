#pragma once 

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "../util/util.hpp"

#define DEBUG         0
#define BUFF_SIZE     1024
#define MAX_EVENT_NUM 1024
class common
{
public:
    /* desc: 构造函数
     * */
    common(int sockfd = -1)
        : _sockfd(sockfd)
        , _sockAddrLen(sizeof(sockaddr_in))
    {
        bzero(&_sockAddr, _sockAddrLen);
    }


    /* desc: 析构函数 */
    virtual ~common()
    {
        if(_sockfd != 0)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    /* desc: 创建监听socket 
     * ret val: 
     *       成功返回 true 
     *       失败返回 false
     * */
    bool createSocket(bool addrReuse = true)
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(_sockfd < 0)
        {
            LOG(util::ERROR) << "socket() error" << std::endl;
            return false;
        }
        if(addrReuse)
        {
            socklen_t opt = 1;
            /* int setsockopt(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) */
            int ret = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(socklen_t));
            if(ret < 0)
            {
                LOG(util::ERROR) << "setsockopt() error" << std::endl;
                return false;
            }
        }
        return true;
    }

    /* 绑定地址信息 */
    bool bind(in_addr_t ip = INADDR_ANY, int port = 2000)
    {
        _sockAddr.sin_family = AF_INET;
        _sockAddr.sin_addr.s_addr = ip;
        _sockAddr.sin_port = htons(port);
        socklen_t sockAddrLen = sizeof(sockaddr_in);
        int ret = ::bind(_sockfd, (sockaddr*)&_sockAddr, sockAddrLen);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::bind() error" << std::endl;
            return false;
        }
        return true;
    }

    /* desc: 设置为监听状态 
     * ret val: 
     *       成功返回 true
     *       失败返回 false
     * */
    bool listen( int backlog = 5 )
    {
        int ret = ::listen(_sockfd, backlog);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::listen() error" << std::endl;
            return false;
        }

        return true;
    }

    
    /* desc: 从连接socket上接收数据
     * ret val:
     *       成功返回 
     * */

    ssize_t recv(char* buf, size_t bufSize)
    {
        memset(buf, 0x00, bufSize);
        ssize_t ret = ::recv(_sockfd, buf, bufSize, 0);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::recv() error" << std::endl;
            return ret;
        }
        return ret;
    }

    ssize_t send(const char * buf, size_t bufSize)
    {
        ssize_t ret = ::send(_sockfd, buf, bufSize, 0);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::send() error" << std::endl;
            return ret;
        }
        return ret;
    }

protected:
    int          _sockfd;
    sockaddr_in  _sockAddr;
    socklen_t    _sockAddrLen;
};


class Epoll
{
public:
    /* 将描述符设置为非阻塞 */
    static int setNonBlocking(int fd)
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

};

class Shell
{
#define IN 1
#define OUT 0

    void readCommand()
    {
        char buf[1024] = {0};
        while(1)
        {
            // 1. 读取用户输入的指令
            printf("[zbq@localhost]$ ");
            fflush(stdout);
            scanf("%[^\n]%*c", buf); // %[] 表示读一个字符串 按[]里规定的内容读 ^\n 表示不是换行符的东西我都读
            // %*c 表示读一个字符并把它丢弃
            // 2. 解析用户输入的指令
            // 假设用户输入的是 ls -l .
            // 解析之后的结果期望是一个字符串数组，数组元素为：ls, -l, .
            char * argv[100] = {0};
            ParseArg(buf, argv);
            // 3. 创建子进程
            //    子进程进行程序替换
            //    父进程进行进程等待
            Run(argv);

        }
    }

    void ParseArg(char input[], char * output[])
    {
        int argc = 0;
        int flag = OUT;
        int i = 0;
        for(i = 0; input[i] != '\0'; ++i)
        {
            if(!isspace(input[i]) && flag == OUT)
            {
                flag = IN;
                output[argc++] = &input[i];
            }
            else if(isspace(input[i]))
            {
                flag = OUT;
                input[i] = '\0';
            }
        }
        output[argc] = NULL;
    }

    int Run(char *argv[])
    {
        pid_t pid = fork();
        if(pid == 0)
        {
            /* 这里需要处理一下 提升一下安全性 
             * 假如用户输入： rm -rf / *
             * */
            execvp(argv[0], argv);
            printf("command %s not found\n", argv[0]);
            exit(1);
        }
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
};
