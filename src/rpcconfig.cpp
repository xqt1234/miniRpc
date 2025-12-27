#include "rpcconfig.h"
#include <fstream>
#include <iostream>
using namespace miniRpc;
bool RpcConfig::loadConfig(std::string filename)
{
    std::ifstream ss(filename);
    if (!ss.is_open())
    {
        //LOG_INFO("找不到配置文件{}",filename);
        std::cout << "找不到配置文件" << filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(ss, line))
    {
        Trim(line);
        if (line[0] == '#' || line.empty())
        {
            continue;
        }
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos)
        {
            //LOG_ERROR("配置行不正确{}");
            std::cout << "配置行不正确" << std::endl;
            continue;
        }
        std::string tmpkey = line.substr(0, eq_pos);
        std::string tmpval = line.substr(eq_pos + 1);
        Trim(tmpkey);
        Trim(tmpval);
        m_configMap.insert({tmpkey, tmpval});
    }
    // for(auto& [key,val]: m_configMap)
    // {
    //     std::cout << "key" << key << "val:" << val << " ";
    // }
    return true;
}

void RpcConfig::Trim(std::string &str)
{
    size_t start_pos = str.find_first_not_of(" ");
    if (start_pos != std::string::npos)
    {
        str = str.substr(start_pos);
    }
    start_pos = str.find_last_not_of(" ");
    if (start_pos != std::string::npos)
    {
        str = str.substr(0, start_pos + 1);
    }
}

std::string RpcConfig::getValue(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it != m_configMap.end())
    {
        return it->second;
    }
    std::cout << "读取配置出错" << key << std::endl;
    //LOG_FATAL("读取配置{}出错", key);
    return "";
}
RpcConfig::RpcConfig()
{
    // loadConfig("rpc.ini");
}
