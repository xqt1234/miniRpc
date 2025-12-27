#include "connectionPool.h"
#include <iostream>
#include "InetAddress.h"
#include "TcpConnection.h"
#include <algorithm>
#include <ranges>
#include <string_view>
#include <functional>
using namespace mymuduo;
using namespace miniRpc;
ConnectionPool::ConnectionPool(std::shared_ptr<ThreadPool> pool, std::shared_ptr<ZkClient> zk)
    : m_pool(pool), m_zk(zk), m_loop(std::make_unique<EventLoop>())
    ,m_client(nullptr)
{
    m_activeClientMap.clear();
    m_allClientMap.clear();
    m_thread = std::thread(&ConnectionPool::startLoop, this);
    m_zk->setNodeUpdateCallBack(std::bind(&ConnectionPool::updateClients, this, std::placeholders::_1));
    m_zk->setWatch("/services");
    updateClients("/services");
}

ConnectionPool::~ConnectionPool()
{
    m_stop = true;
    // if(m_client)
    // {
    //     m_client->disconnect();
    //     delete m_client;
    // }
    std::cout << "当前有连接个数" << m_activeClientMap.size() << std::endl;
    for(auto& val : m_allClientMap)
    {
        std::cout << val.first << std::endl;
        for(auto& conn:val.second)
        {
            // std::cout << conn->name() << std::endl;
            std::cout << "断开连接" << std::endl;
            conn->disconnect();
            //delete conn;
        }
    }
    m_activeClientMap.clear();
    m_allClientMap.clear();
    //m_AllclientMap.clear();
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
    for(auto& val : m_activeClientMap)
    {
        std::cout << "当前有服务:" << val.first << std::endl;
    }
    auto it = m_activeClientMap.find(servicename);
    if (it != m_activeClientMap.end())
    {
        if (it->second.size() > 0)
        {
            int current = m_currentUse[servicename]++;
            int count = it->second.size();
            std::shared_ptr<TcpClient> res = it->second[current & count];
            count++;
            return it->second[current % count];
        }
    }
    return nullptr;
}

void ConnectionPool::checkClients()
{
    std::cout << "打印客户端" << std::endl;
    for (auto &val : m_activeClientMap)
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
        // 查找服务
        checkService();
    }
    else if (nodes.size() == 3)
    {
        // 根据服务名，更新客户端列表，这里是服务名称
        std::cout <<"nodes[2]:" << nodes[2] << std::endl;
        updateClient(nodes[2]);
    }
}

void ConnectionPool::setMessageCallBack(std::function<void(const std::string&)> cb)
{
    m_msgCallBack = cb;
}

void ConnectionPool::startLoop()
{
    std::cout << "eventloop启动...." << std::endl;
    m_loop->loop();
}

void ConnectionPool::updateClient(std::string servicename)
{
    std::vector<std::shared_ptr<TcpClient>> &clients = m_allClientMap[servicename];
    std::string path = "/services/" + servicename;
    std::vector<std::string> res = m_zk->getNodeChildren(path);
    // std::vector<std::shared_ptr<mymuduo::TcpClient>> &tmpclients = m_clientMap[servicename];
    std::cout << "打印服务全称" << path << std::endl;
    for (auto &val : res)
    {
        std::cout << "获取子节点为:" << val << std::endl;
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
            std::cout << "准备创建节点:" << val << clientname << servicename << std::endl;
            createTcpClient(val, clientname, servicename);
        }
    }
}
void ConnectionPool::createTcpClient(const std::string &ipPort, const std::string &clientName, const std::string &servicename)
{
    std::vector<std::shared_ptr<TcpClient>> &tvec = m_allClientMap[servicename];
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
    std::cout << "ip地址是:" << iaddr.toIpPortString() << std::endl;
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(m_loop.get(), iaddr, clientName);
    client->setConnectionCallBack(std::bind(&ConnectionPool::newConnection,this,std::placeholders::_1));
    client->setMessageCallBack(std::bind(&ConnectionPool::onMessage,this,std::placeholders::_1,std::placeholders::_2));
    client->connect(true);
    tvec.push_back(client);
}

void ConnectionPool::sendHeart()
{
    std::cout << "sendHeart被调用" << std::endl;
}

void ConnectionPool::newConnection(const TcpConnectionPtr &conn)
{
    if(conn->isConnected())
    {
        for(auto& [servicename,vec] : m_allClientMap)
        {
            std::cout << "服务名---" << servicename << std::endl;
            for(auto& client: vec)
            {
                if(client->connection() == conn)
                {
                    std::cout << "找到相同的了" << std::endl;
                    m_activeClientMap[servicename].push_back(client);
                }
            }
        }

        std::cout << "tcpclient建立了连接" << std::endl;
    }else
    {
        for(auto& [servicename,vec] : m_activeClientMap)
        {
            for(auto it = vec.begin();it != vec.end();++it)
            {
                if((*it)->connection() == conn)
                {
                    vec.erase(it);
                }
            }
        }
        std::cout << "tcpclient断开了连接" << std::endl;
    }
}

void ConnectionPool::onMessage(const TcpConnectionPtr &conn, Buffer *buffer)
{
    std::string msg = buffer->readAllAsString();
    if(m_msgCallBack)
    {
        m_msgCallBack(msg);
    }
    // conn->send(msg);
}
void ConnectionPool::checkService()
{
    std::string rootPath = "/services";
    std::vector<std::string> res = m_zk->getNodeChildren(rootPath);
    std::cout << "打印服务名" << std::endl;
    for (auto &val : res)
    {
        std::cout << "checkService():" << val << std::endl;
        if (m_allClientMap.count(val) == 0)
        {
            std::string servicename = "/services/" + val;
            m_zk->setWatch(servicename);
            // 更新服务
            updateClients(servicename);
        }
    }
}
