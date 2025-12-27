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
namespace miniRpc
{
    class RpcChannel
    {
    public:
    private:
        std::shared_ptr<ThreadPool> m_threadPool;
        std::shared_ptr<ConnectionPool> m_connPool;
        std::shared_ptr<ZkClient> m_client;
        std::unordered_map<int, std::function<void(std::string)>> m_requestMap;
        std::atomic<int64_t> m_requestId{0};

    public:
        RpcChannel();
        ~RpcChannel() = default;
        void callMethodAsync(const std::string &serviceName,
                             const std::string &methodName, const std::string &request,
                             std::function<void(std::string)>);
        void getResponse(const std::string &response);
    };
}
