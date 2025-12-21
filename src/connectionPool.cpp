#include "connectionPool.h"
#include <iostream>
#include "InetAddress.h"
#include "TcpConnection.h"
#include <algorithm>
#include <ranges>
#include <string_view>
using namespace mymuduo;
ConnectionPool::ConnectionPool(std::shared_ptr<ThreadPool> pool, std::shared_ptr<ZkClient> zk)
    : m_pool(pool), m_zk(zk), m_loop(std::make_unique<EventLoop>())
{
    m_thread = std::thread(&ConnectionPool::startLoop, this);
    m_zk->setNodeUpdateCallBack(std::bind(&ConnectionPool::updateClients, this, std::placeholders::_1));
    m_zk->setWatch("/services");
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
std::shared_ptr<mymuduo::TcpClient> ConnectionPool::getConnection(const std::string &servicename)
{
    auto it = m_clientMap.find(servicename);
    if (it != m_clientMap.end())
    {
        if (it->second.size() > 0)
        {
            int current = m_currentUse[servicename]++;
            int count = it->second.size();
            return it->second[current % count];
        }
    }
    return std::shared_ptr<TcpClient>();
}

void ConnectionPool::initNode()
{
    updateClients("/services");
}

void ConnectionPool::checkClients()
{
    std::cout << "打印客户端" << std::endl;
    for (auto &val : m_clientMap)
    {
        std::cout << val.first << std::endl;
    }
}
void ConnectionPool::updateClients(const std::string &path)
{
    auto parts = path | std::ranges::views::split('/');
    std::vector<std::string> nodes;
    for (auto range : parts)
    {
        nodes.emplace_back(range.begin(), range.end());
    }
    for (auto &val : nodes)
    {
        std::cout << "节点：：：：" << val << std::endl;
    }
    if (nodes.size() == 2 && nodes[1] == "services")
    {
        checkService();
    }
    else if (nodes.size() == 3)
    {
        updateClient(nodes[2]);
    }
}

void ConnectionPool::startLoop()
{
    m_loop->loop();
}

void ConnectionPool::updateClient(std::string servicename)
{
    std::vector<std::shared_ptr<mymuduo::TcpClient>> &clients = m_clientMap[servicename];
    std::string path = "/services/" + servicename;
    std::vector<std::string> res = m_zk->getNodeChildren(path);
    std::vector<std::shared_ptr<mymuduo::TcpClient>> &tmpclients = m_clientMap[servicename];
    std::cout << "打印服务ip地址" << std::endl;
    for (auto &val : res)
    {
        std::string clientname = servicename + val;
        bool hasClient = false;
        for (auto &p : clients)
        {
            if (p->name() == clientname)
            {
                hasClient = true;
                break;
            }
        }
        // 不存在，创建tcpclient
        if (!hasClient)
        {
            createTcpClient(val, clientname, val);
        }
    }
}
void ConnectionPool::createTcpClient(const std::string &ipPort, const std::string &clientName, const std::string &servicename)
{
    std::vector<std::shared_ptr<TcpClient>> &tvec = m_AllclientMap[servicename];
    for(auto& val : tvec)
    {
        if(val->name() == clientName)
        {
            // 有正在重连的，等待重连即可。
            return;
        }
    }
    int index = ipPort.find(":");
    std::string addr = ipPort.substr(0, index);
    int port = atoi(ipPort.substr(index + 1).c_str());
    InetAddress iaddr(port, addr);

    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(m_loop.get(), iaddr, clientName);
    client->setConnectionCallBack([this, clientName, client](const TcpConnectionPtr &conn) mutable
                                  {
                    if (conn->isConnected()) {
                        m_clientMap[clientName].push_back(client);
                        
                    } else {
                        std::vector<std::shared_ptr<TcpClient>>& tvec = m_clientMap[clientName];
                        auto it = std::find(tvec.begin(),tvec.end(),client);
                        if(it != tvec.end())
                        {
                            tvec.erase(it);
                        }
                        // 断开自动重连
                        client->connect();
                    } });
    client->setRetry(true);
    client->connect();
    tvec.push_back(client);
}

void ConnectionPool::sendHeart()
{

}

void ConnectionPool::checkService()
{
    std::string rootPath = "/services";
    std::vector<std::string> res = m_zk->getNodeChildren(rootPath);
    std::cout << "打印服务名" << std::endl;
    for (auto &val : res)
    {
        if (m_clientMap.count(val) == 0)
        {
            m_zk->setWatch("/services/" + val);
            m_clientMap.insert({val, std::vector<std::shared_ptr<TcpClient>>()});
            updateClient(val);
        }
    }
}
