#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "threadpool.h"
#include "rpcService.h"
#include "TcpServer.h"
#include "TcpConnection.h"
namespace miniRpc
{
    class ProVider
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<RpcService>> m_serviceMap;
        // std::shared_ptr<ZkClient> m_zk;
        // std::shared_ptr<ThreadPool> m_pool;
        std::string m_rootLoc;
        std::unique_ptr<mymuduo::EventLoop> m_loop;
        std::unique_ptr<mymuduo::TcpServer> m_server;
        std::string m_rpcIp;
        int16_t m_rpcPort;
    public:
        ProVider();
        ~ProVider();
        void start();
        void AddService(std::shared_ptr<RpcService> service);
        // bool callAsyncServiceMethod(const std::string& servicename,const std::string& methodname,const std::string& request,std::function<void(std::string)>);
        // ThreadPool m_threadPool;
        void onMessage(const mymuduo::TcpConnectionPtr &conn, mymuduo::Buffer *buffer);
        void onConnection(const mymuduo::TcpConnectionPtr &conn);
    private:
        void processReq(const mymuduo::TcpConnectionPtr &conn,const std::string& req);
    };
}
