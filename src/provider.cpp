#include "provider.h"
#include "EventLoop.h"
ProVider::ProVider()
{
    
}

void ProVider::start()
{
    EventLoop loop;
    TcpServer server(&loop,10002,"192.168.105.2");
    server.setThreadNum(3);
    server.setMessageCallBack(std::bind(&ProVider::onMessage,this,std::placeholders::_1,std::placeholders::_2));
    server.setConnectionCallBack(std::bind(&ProVider::onConnection,this,std::placeholders::_1));

}

void ProVider::AddService(std::shared_ptr<RpcService> service)
{
    m_serviceMap[service->m_name] = service;
    
}

void ProVider::onMessage(const TcpConnectionPtr& conn,Buffer* buffer)
{

}

void ProVider::onConnection(const TcpConnectionPtr &conn)
{
    if(!conn->isConnected())
    {
        std::cout << "连接断开" << std::endl;
        conn->shutdown();
    }else
    {
        std::cout << "有客户端连接" << std::endl;
    }
}

// bool ProVider::callAsyncServiceMethod(const std::string &servicename,const std::string &methodname,const std::string &request, std::function<void(std::string)> func)
// {
//     auto it = m_serviceMap.find(servicename);
//     if (it == m_serviceMap.end())
//     {
//         return false;
//     }
//     it->second->CallAsyncMethod(methodname, request, func);
//     // std::shared_ptr<RpcService> service = it->second;
    
//     // //m_threadPool.addTask(it->second->CallAsyncMethod(methodname, request, func));
//     // m_threadPool.addTask([service,methodname,request,func]{
//     //     service->CallAsyncMethod(methodname,request,func);
//     // });
//     return true;
// }
