#pragma once
#include <string>
#include <functional>
#include "threadpool.h"
#include "zkclient.h"
#include "TcpClient.h"
#include "connectionPool.h"
#include <unordered_map>
class RpcChannel
{
private:
    std::shared_ptr<ThreadPool> m_threadPool;
    std::shared_ptr<ConnectionPool> m_connPool;
    std::shared_ptr<ZkClient> m_client;
public:
    RpcChannel();
    ~RpcChannel() = default;
    void callMethodAsync(const std::string& serviceName,
        const std::string& methodName,const std::string& request,
    std::function<void(const std::string&)> callback);

};
