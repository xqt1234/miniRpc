#include "connectionPool.h"
#include <iostream>
#include "InetAddress.h"
#include "TcpConnection.h"
#include <algorithm>
ConnectionPool::ConnectionPool(std::shared_ptr<ThreadPool> pool, std::shared_ptr<ZkClient> zk)
    : m_pool(pool), m_zk(zk), m_loop(std::make_unique<EventLoop>())
{
    m_thread = std::thread(&ConnectionPool::startLoop, this);
}
ConnectionPool::~ConnectionPool()
{
    if (m_loop != nullptr)
    {
        m_loop->quit();
    }
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
std::shared_ptr<TcpClient> ConnectionPool::getConnection(const std::string &servicename)
{
    auto it = m_clientMap.find(servicename);
    if(it != m_clientMap.end())
    {
        if(it->second.size() > 0)
        {
            // TODO 待添加选取逻辑
            return it->second[0];
        }
    }
    return std::shared_ptr<TcpClient>();
}

void ConnectionPool::initNode()
{
    std::string rootPath = "/services";
    std::vector<std::string> res = m_zk->getNodeChildren(rootPath);
    std::cout << "节点个数是:" << res.size() << std::endl;
    for (auto &serviceName : res)
    {
        std::cout << "子节点是：" << serviceName << std::endl;
        std::string hoststr = rootPath + "/" + serviceName;
        std::vector<std::string> hosts = m_zk->getNodeChildren(hoststr);
        std::vector<std::shared_ptr<TcpClient>> vec;
        for (auto &host : hosts)
        {
            int index = host.find(":");
            std::string addr = host.substr(0, index);
            int port = atoi(host.substr(index + 1).c_str());
            InetAddress iaddr(port, addr);
            
            std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(m_loop.get(), iaddr, host);
            client->setConnectionCallBack([this, serviceName, client](const TcpConnectionPtr &conn) mutable{
                    if (conn->isConnected()) {
                        m_clientMap[serviceName].push_back(client);
                    } else {
                        std::vector<std::shared_ptr<TcpClient>>& tvec = m_clientMap[serviceName];
                        auto it = std::find(tvec.begin(),tvec.end(),client);
                        if(it != tvec.end())
                        {
                            tvec.erase(it);
                        }
                    } });
            client->connect();
            vec.push_back(client);
        }
        m_AllclientMap.insert({serviceName,vec});
    }
}

void ConnectionPool::checkClients()
{
    std::cout << "打印客户端" << std::endl;
    for(auto& val : m_clientMap)
    {
        std::cout << val.first << std::endl;
    }
}

void ConnectionPool::startLoop()
{
    m_loop->loop();
}
