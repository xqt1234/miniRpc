#include "rpcService.h"
using namespace miniRpc;
RpcService::RpcService()
{
    
}

RpcService::~RpcService()
{
    m_ansyncMethodMap.clear();
    m_methodMap.clear();
}

const std::string &RpcService::getname() const
{
    return m_name;
}

void RpcService::setServiceName(const std::string &name)
{
    m_name = name;
}

void RpcService::addMethod(const std::string &name, RpcMethod method)
{
    m_methodMap[name] = std::move(method);
}

void RpcService::addAsyncMethod(const std::string &name, RpcAsyncMethod method)
{
    m_ansyncMethodMap[name] = std::move(method);
}

bool RpcService::CallMethod(const std::string &methodname, const std::string &request, std::string &response)
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
