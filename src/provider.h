#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "threadpool.h"
#include "rpcService.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
namespace miniRpc
{
    class ProVider
    {
    private:
        struct ConnectionInfo
        {
            mymuduo::TcpConnectionPtr conn;
            int64_t requestId;
            std::shared_ptr<google::protobuf::Message> response;
        };
        std::unordered_map<std::string, google::protobuf::Service *> m_serviceMap;
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
        void AddService(google::protobuf::Service *service);
        // bool callAsyncServiceMethod(const std::string& servicename,const std::string& methodname,const std::string& request,std::function<void(std::string)>);
        // ThreadPool m_threadPool;
        void onMessage(const mymuduo::TcpConnectionPtr &conn, mymuduo::Buffer *buffer);
        void onConnection(const mymuduo::TcpConnectionPtr &conn);

    private:
        void processReq(const mymuduo::TcpConnectionPtr &conn, const std::string &req, int64_t requestId);
        void handSend(google::protobuf::Message*, std::shared_ptr<ConnectionInfo> info);
    };
}
