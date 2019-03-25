#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <sys/time.h>

namespace util
{
    class timeUtil
    {
    public:
        static int64_t getTimeStamp()
        {
            int64_t ret = 0;
            timeval tv;
            gettimeofday(&tv, nullptr);
            ret = tv.tv_sec + tv.tv_usec/1000;

            return ret;
        }
    };
    
    enum logLevel
    {
        INFO,
        WARNING,
        ERROR,
        FATAL
    };

    inline std::ostream& log(logLevel level, std::string fileName, int fileNo)
    {
        std::string prefix;
        prefix += '[';
        if(level == INFO)
        {
            prefix += 'I';
        }
        else if (level == WARNING)
        {
            prefix += 'W';
        }
        else if (level == ERROR)
        {
            prefix += 'E';
        }
        else if (level == FATAL)
        {
            prefix += 'F';
        }
        else
        {
            prefix += 'U';// 未知错误
        }
        prefix += std::to_string(timeUtil::getTimeStamp());
        prefix += ' ';
        prefix += fileName;
        prefix += ':';
        prefix += std::to_string(fileNo);
        prefix += ']';
        std::cout << prefix;

        return std::cout;
    }

#define LOG(level) log(level, __FILE__, __LINE__)
    class fileUtil
    {
    public:
        static bool read(std::string filePath, std::string * content)
        {
            std::ifstream file(filePath.c_str());
            (*content).clear();
            if(!file.is_open())
            {
                return false;
            }
            std::string line;
            while(std::getline(file, line))
            {
                *content += line + "\n";
            }
            file.close();
            return true;
        }

        static bool write(std::string filePath, const std::string& content)
        {
            std::ofstream file(filePath.c_str());
            if(!file.is_open())
            {
                return false;
            }
            file.write(content.c_str(), content.size());
            return true;
        }

    };

    class stringUtil
    {
    public:
        static void split(const std::string& input, const std::string& splitChar, std::vector<std::string>* output)
        {
            boost::split(*output, input, boost::is_any_of(splitChar), boost::token_compress_on);
        }
    };

    class urlUtil
    {
    public:
        static void parseBody(const std::string& body, std::unordered_map<std::string, std::string>* params)
        {
            // 1. 先对body字符串进行切分，切分成键值对的形式
            //   a) 对按照&符号切分
            //   b) 在按照=切分
            // 2. 对这里的键值对进行urldecode
            std::vector<std::string> kvs;
            stringUtil::split(body, "&", &kvs);
            for(size_t i = 0; i < kvs.size(); ++i)
            {
                std::vector<std::string> kv;
                stringUtil::split(kvs[i], "=", &kv);
                if(kv.size() != 2)
                    continue;
                (*params)[kv[0]] = urlDecode(kv[1]);
            }
        }


        static unsigned char toHex(unsigned char x)
        {
            return x > 9 ? x+55 : x + 48;
        }

        static unsigned char fromHex(unsigned char x)
        {
            unsigned char y = 0;
            if(x >= 'A' && x <= 'Z')
                y = x -'A' + 10;
            else if(x >= 'a' && x <= 'z')
                y = x - 'a' + 10;
            else if(x >= '0' && x <= '9')
                y = x - '0';
            else 
                assert(0);

            return y;
        }

        static std::string urlEncode(const std::string& str)
        {
            std::string strTemp = "";
            size_t length = str.length();
            for(size_t i = 0; i < length; ++i)
            {
                if(isalnum((unsigned char)str[i]) ||
                   (str[i] == '-') ||
                   (str[i] == '_') ||
                   (str[i] == '.') ||
                   (str[i] == '~'))
                {
                    strTemp += str[i];
                }
                else if(str[i] == ' ')
                {
                    strTemp += '+';
                }
                else
                {
                    strTemp += '%';
                    strTemp += toHex((unsigned char)str[i]>>4);
                    strTemp += toHex((unsigned char)str[i]%16);
                }
            }
            return strTemp;
        }

        static std::string urlDecode(const std::string& str)
        {
            std::string strTemp = "";
            size_t length = str.length();
            for(size_t i = 0; i < length; ++i)
            {
                if(str[i] == '+')
                    strTemp += ' ';
                else if(str[i] == '%')
                {
                    assert(i+2 < length);
                    unsigned char high = fromHex((unsigned char)str[++i]);
                    unsigned char low = fromHex((unsigned char)str[++i]);
                    strTemp += high*16 + low;
                }
                else
                {
                    strTemp += str[i];
                }
            }
            return strTemp;
        }
    };

}// namespace util
