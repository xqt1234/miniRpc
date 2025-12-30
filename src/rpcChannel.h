#pragma once
#include <string>
#include <functional>
#include "threadpool.h"
#include "zkclient.h"
#include "TcpClient.h"
#include "connectionPool.h"
#include <unordered_map>
#include <functional>
#include <atomic>
#include "public.h"
namespace miniRpc
{
    class RpcChannel
    {
    public:
        
    private:
        std::shared_ptr<ConnectionPool> m_connPool;
        std::unordered_map<int, std::function<void(std::string)>> m_requestMap;
        
    public:
        RpcChannel();
        ~RpcChannel();
        void callMethodAsync(const std::string &serviceName,
                             const std::string &methodName, const std::string &request,
                             std::function<void(std::string)>);
        void getResponse(Buffer* buff);
    };
}
