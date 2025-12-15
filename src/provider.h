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
    
public:
    ProVider(/* args */);
    ~ProVider() = default;
    void start();
    void AddService(std::shared_ptr<RpcService> service);
    //bool callAsyncServiceMethod(const std::string& servicename,const std::string& methodname,const std::string& request,std::function<void(std::string)>);
    //ThreadPool m_threadPool;
    void onMessage(const TcpConnectionPtr& conn,Buffer* buffer);
    void onConnection(const TcpConnectionPtr& conn);
};
