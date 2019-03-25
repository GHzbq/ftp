#include "../common/common.hpp"

/* 这个类为server的accept函数返回值专门创建的socket 
 * 由于这个服务器的功能目前只有文件传输
 * 所有没加区分
 * */
class ftpSock
{
public: 
    ftpSock(int sockfd, sockaddr_in& sockAddr)
        : _sockfd(sockfd)
        , _sockAddr(sockAddr)
        , _sockAddrLen(sizeof(sockaddr_in))
    {}

    ~ftpSock()
    {
        // 这里可能存在线程安全问题
        // 之后再考虑吧
        // 目前先用多进程处理
        if(_sockfd != -1)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    bool recv(char* buf, size_t bufSize)
    {
        ssize_t ret = ::recv(_sockfd, buf, bufSize-1, 0);
        if(ret < 0)
        {
            LOG(util::ERROR) << "recv() error" << std::endl;
            return false;
        }
        else if(ret == 0)
        {
            LOG(util::INFO) << "peer shutdown" << std::endl;
            return false;
        }
        else
        {
            buf[ret] = '\0';
            std::cout << buf << std::endl;
        }
        return true;
    }

    bool send(const char * buf, size_t bufSize)
    {
        ssize_t ret = ::send(_sockfd, buf, bufSize, 0);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::send() error" << std::endl;
            return false;
        }
        else if(ret == 0)
        {
            LOG(util::INFO) << "peer shutdown" << std::endl;
            return false;
        }
        return true;
    }
    

    /*  */
    void doServer()
    {
        char buf[BUFF_SIZE] = {0};
        while(1)
        {
            std::cout << "ftp> " ;
            std::cout.flush();
            recv(buf, BUFF_SIZE);
            std::string resp = "ceshiyiba";
            send(resp.c_str(), resp.size());

        }
    }

private:
    int         _sockfd;
    sockaddr_in _sockAddr;
    socklen_t   _sockAddrLen;
};

class Server : public common
{
public:
    Server()
    {
        createSocket();
    }

    /* desc: 接收客户端请求
     * ret val: 
     *       成功返回 连接socket
     *       失败返回 -1
     * */
    int accept(sockaddr_in * clientAddr, socklen_t * clientAddrLen)
    {
        int ret = ::accept(_sockfd, (sockaddr*)clientAddr, clientAddrLen);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::accept() error" << std::endl;
            return ret;
        }
        return ret;
    }
};

