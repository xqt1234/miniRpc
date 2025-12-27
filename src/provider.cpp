#include "provider.h"
#include "EventLoop.h"
#include "Logger.h"
#include "rpcApplication.h"
using namespace mymuduo;
using namespace miniRpc;
miniRpc::ProVider::ProVider()
{
    RpcConfig& conf = RpcApplication::getRpcConfig();
    m_rpcIp = RpcApplication::getRpcConfig().getValue("rpcserverip");
    m_rpcPort = atoi(RpcApplication::getRpcConfig().getValue("rpcserverport").c_str());
    start();
}

ProVider::~ProVider()
{
    if (m_loop)
    {
        m_loop->quit();
    }
}

void ProVider::start()
{
    m_rootLoc = "/services";
    bool res = RpcApplication::getZkClient().createNode(m_rootLoc, "", ZOO_PERSISTENT);
    if (!res)
    {
        LOG_FATAL("zk根路径创建失败");
    }
    m_loop = std::make_unique<EventLoop>();
    m_server = std::make_unique<TcpServer>(m_loop.get(), m_rpcPort, m_rpcIp);
    m_server->setThreadNum(3);
    m_server->setMessageCallBack(std::bind(&ProVider::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_server->setConnectionCallBack(std::bind(&ProVider::onConnection, this, std::placeholders::_1));
    m_server->start();
    RpcApplication::getThreadPool().addTask([&]
                    { m_loop->loop(); });
}

void ProVider::AddService(std::shared_ptr<RpcService> service)
{
    m_serviceMap[service->m_name] = service;
    std::cout << "----节点名称:" << service->m_name << std::endl;
    std::string loc = m_rootLoc + "/" + service->m_name;
    ZkClient& zk = RpcApplication::getZkClient();
    bool res = zk.createNode(loc, "", ZOO_PERSISTENT);
    std::string ip = m_rpcIp + ":" + std::to_string(m_rpcPort);
    std::string host = loc + "/" + ip;
    if (res)
    {
        std::vector<std::string> nodes = zk.getNodeChildren(m_rootLoc);
        std::cout << "节点个数为:" << nodes.size() << std::endl;
        res = zk.createNode(host, "", ZOO_EPHEMERAL);
        if (!res)
        {
            std::cout << "创建节点失败" << std::endl;
        }
        else
        {
            std::cout << "创建节点成功" << host << std::endl;
        }
    }
}

void ProVider::onMessage(const TcpConnectionPtr &conn, Buffer *buffer)
{
    std::string msg = buffer->readAllAsString();
    std::cout <<"收到消息:" << msg << std::endl;
    conn->send(msg);
}

void ProVider::onConnection(const TcpConnectionPtr &conn)
{
    if (!conn->isConnected())
    {
        std::cout << "连接断开" << std::endl;
        // conn->shutdown();
    }
    else
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
