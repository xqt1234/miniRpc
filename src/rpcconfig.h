#pragma once
#include <string>
#include <unordered_map>
namespace miniRpc
{
    class RpcConfig
    {
    private:
        std::unordered_map<std::string, std::string> m_configMap;

    public:
        RpcConfig(/* args */);
        ~RpcConfig() = default;
        void Trim(std::string &str);
        std::string getValue(const std::string &key);
        bool loadConfig(std::string filename);
    };
}
