#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "zkclient.h"
#include "threadpool.h"
using RpcMethod = std::function<void(const std::string request, const std::string &response)>;
using RpcAsyncMethod = std::function<void(const std::string &request, std::function<void(std::string &response)> callback)>;
namespace miniRpc
{
    class RpcService
    {
    private:
        std::string m_name;
        std::unordered_map<std::string, RpcMethod> m_methodMap;
        std::unordered_map<std::string, RpcAsyncMethod> m_ansyncMethodMap;
        friend class ProVider;

    public:
        RpcService();
        ~RpcService();
        const std::string &getname() const;
        void setServiceName(const std::string &name);
        void addMethod(const std::string &name, RpcMethod method);
        void addAsyncMethod(const std::string &name, RpcAsyncMethod method);
        bool CallMethod(const std::string &methodname, const std::string &request, std::string &response);
        bool CallAsyncMethod(const std::string &methodname, const std::string &request, std::function<void(std::string)> done);
    };
}
