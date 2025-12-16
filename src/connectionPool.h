#pragma once
#include "TcpClient.h"
#include <unordered_map>
#include "zkclient.h"
#include <thread>
#include <atomic>
#include "EventLoop.h"
#include <vector>
class ConnectionPool
{
private:
    std::unordered_map<std::string,std::vector<std::shared_ptr<TcpClient>>> m_AllclientMap;
    std::unordered_map<std::string,std::vector<std::shared_ptr<TcpClient>>> m_clientMap;
    std::shared_ptr<ThreadPool> m_pool;
    std::shared_ptr<ZkClient> m_zk;
    std::thread m_thread;
    std::atomic<bool> m_stop{false};
public:
    ConnectionPool(std::shared_ptr<ThreadPool> pool,std::shared_ptr<ZkClient> zk);
    ~ConnectionPool();
    std::shared_ptr<TcpClient> getConnection(const std::string& servicename);
    void initNode();
    void checkClients();
private:
    void startLoop();
    std::unique_ptr<EventLoop> m_loop;
};

