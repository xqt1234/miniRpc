#pragma once
#include "TcpClient.h"
#include <unordered_map>
#include "zkclient.h"
class ConnectionPool
{
private:
    std::unordered_map<std::string,std::unique_ptr<TcpClient>> m_clientMap;
    ZkClient m_zk;
    std::shared_ptr<ThreadPool> m_pool;
public:
    ConnectionPool(std::shared_ptr<ThreadPool> pool);
    ~ConnectionPool() = default;
    std::unique_ptr<TcpClient> getConnection(const std::string& servicename);
private:
    void initZkNode();
};

