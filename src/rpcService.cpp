#include "rpcService.h"

RpcService::RpcService(std::shared_ptr<ThreadPool> pool)
{
    m_pool = pool;
    m_zk = std::make_unique<ZkClient>(pool);
}

void RpcService::addMethod(std::string &name, RpcMethod method)
{
    m_methodMap[name] = std::move(method);
}

void RpcService::addAsyncMethod(std::string &name, RpcAsyncMethod method)
{
    m_ansyncMethodMap[name] = std::move(method);
    std::string loc = "/services/" + name;
    std::string host = loc + "/" + "192.168.105.2:10003";
    m_zk->createNode(loc,"");
    m_zk->createNode(host,"");
}

bool RpcService::CallMethod(std::string &methodname, const std::string &request, std::string &response)
{
    auto it = m_methodMap.find(methodname);
    if (it != m_methodMap.end())
    {
        it->second(request, response);
        return true;
    }
    return false;
}

bool RpcService::CallAsyncMethod(const std::string &methodname, const std::string &request, std::function<void(std::string)> done)
{
    auto it = m_ansyncMethodMap.find(methodname);
    if (it != m_ansyncMethodMap.end())
    {
        RpcAsyncMethod method = it->second;
        method(request, std::move(done));
        return true;
    }
    return false;
}
