#include "server.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " ip + port" << std::endl;
        return 1;
    }

    Server server;
    server.bind(inet_addr(argv[1]), std::stoi(argv[2]));
    server.listen();
    while(1)
    {
        sockaddr_in sockAddr;
        socklen_t sockAddrLen = sizeof(sockaddr_in);
        int sockfd = server.accept(&sockAddr, &sockAddrLen);
        ftpSock sock(sockfd, sockAddr); 
        pid_t pid = fork();
        if(pid < 0)
        {
            LOG(util::ERROR) << "fork() error" << std::endl;
            break;
        }
        else if (pid == 0)
        {
            // 子进程中需要关闭父进程sock，不然就是两个服务器了
            // 子进程负责与客户端通信，传输文件
            server.~Server();
            sock.doServer();
        }
        else
        {
            // 父进程关闭这个连接socket
            sock.~ftpSock();
        }
    }
}
