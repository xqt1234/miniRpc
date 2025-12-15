#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "zkclient.h"
#include "threadpool.h"
using RpcMethod = std::function<void(const std::string request,const std::string& response)>;
using RpcAsyncMethod = std::function<void(const std::string& request, std::function<void(std::string& response)> callback)>;
class RpcService
{
private:
    std::string m_name;
    std::unordered_map<std::string,RpcMethod> m_methodMap;
    std::unordered_map<std::string,RpcAsyncMethod> m_ansyncMethodMap;
    friend class ProVider;
    std::unique_ptr<ZkClient> m_zk;
    std::shared_ptr<ThreadPool> m_pool;
public:
    RpcService(std::shared_ptr<ThreadPool> pool);
    ~RpcService() = default;
    void addMethod(std::string& name,RpcMethod method);
    void addAsyncMethod(std::string& name,RpcAsyncMethod method);
    bool CallMethod(std::string& methodname,const std::string& request,std::string& response);
    bool CallAsyncMethod(const std::string& methodname,const std::string& request,std::function<void(std::string)> done);
};
