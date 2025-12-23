#include "provider.h"
#include "EventLoop.h"
#include "Logger.h"
using namespace mymuduo;

ProVider::ProVider()
    : m_pool(nullptr), m_zk(nullptr)
{
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
    m_pool = std::make_shared<ThreadPool>();
    m_pool->start();
    m_zk = std::make_shared<ZkClient>(m_pool);
    m_zk->reConnect();
    m_rootLoc = "/services";
    bool res = m_zk->createNode(m_rootLoc, "", ZOO_PERSISTENT);
    if (!res)
    {
        LOG_FATAL("zk根路径创建失败");
    }
    m_loop = std::make_unique<EventLoop>();
    m_server = std::make_unique<TcpServer>(m_loop.get(), 10002, "192.168.105.2");
    m_server->setThreadNum(3);
    m_server->setMessageCallBack(std::bind(&ProVider::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    m_server->setConnectionCallBack(std::bind(&ProVider::onConnection, this, std::placeholders::_1));
    m_server->start();
    m_pool->addTask([&]
                    { m_loop->loop(); });
}

void ProVider::AddService(std::shared_ptr<RpcService> service)
{

    m_serviceMap[service->m_name] = service;
    std::cout << "----节点名称:" << service->m_name << std::endl;
    std::string loc = m_rootLoc + "/" + service->m_name;
    bool res = m_zk->createNode(loc, "", ZOO_PERSISTENT);
    std::string host = loc + "/" + "192.168.105.2:10002";
    if (res)
    {
        std::vector<std::string> nodes = m_zk->getNodeChildren(m_rootLoc);
        std::cout << "节点个数为:" << nodes.size() << std::endl;
        res = m_zk->createNode(host, "", ZOO_EPHEMERAL);
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
