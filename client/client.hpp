#include "../common/common.hpp"

class Client : public common
{
public:
    Client()
    {}
    bool connect( const char* ip, int port = 9000)
    {
        _sockAddr.sin_family = AF_INET;
        _sockAddr.sin_addr.s_addr = inet_addr(ip);
        _sockAddr.sin_port = htons(port);
        
        int ret = ::connect(_sockfd, (sockaddr*)&_sockAddr, _sockAddrLen);
        if(ret < 0)
        {
            LOG(util::ERROR) << "::connect() error" << std::endl;
            return false;
        }
        return true;
    }

    int getFd()const
    {
        return _sockfd;
    }
};
