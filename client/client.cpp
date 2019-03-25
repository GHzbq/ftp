#include "client.hpp"

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " ip + port" << std::endl;
        return 1;
    }

    Client client;
    client.createSocket();
    client.connect(argv[1], atoi(argv[2]));

    char buf[BUFF_SIZE] = {0};
    while(1)
    {
        std::cout << "ftp> ";
        std::cout.flush();
        ssize_t ret = read(0, buf, BUFF_SIZE);
        if(ret < 0)
        {
            LOG(util::ERROR) << "read() error" << std::endl;
            break;
        }
        send(client.getFd(), buf, ret, 0);
        memset(buf, 0x00, BUFF_SIZE);
        client.recv(buf, BUFF_SIZE);
        std::cout << buf << std::endl;
    }

    return 0;
}
