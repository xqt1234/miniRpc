#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "threadpool.h"
#include "rpcService.h"
#include "TcpServer.h"
#include "TcpConnection.h"
class ProVider
{
private:
    std::unordered_map<std::string,std::shared_ptr<RpcService>> m_serviceMap;
    std::shared_ptr<ZkClient> m_zk;
    std::shared_ptr<ThreadPool> m_pool;
    std::string m_rootLoc;
    std::unique_ptr<EventLoop> m_loop;
    std::unique_ptr<TcpServer> m_server;
public:
    ProVider(std::shared_ptr<ThreadPool> pool,std::shared_ptr<ZkClient> zk);
    ~ProVider();
    void start();
    void AddService(std::shared_ptr<RpcService> service);
    //bool callAsyncServiceMethod(const std::string& servicename,const std::string& methodname,const std::string& request,std::function<void(std::string)>);
    //ThreadPool m_threadPool;
    void onMessage(const TcpConnectionPtr& conn,Buffer* buffer);
    void onConnection(const TcpConnectionPtr& conn);

};
