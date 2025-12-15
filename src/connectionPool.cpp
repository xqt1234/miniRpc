#include "connectionPool.h"
#include <iostream>
ConnectionPool::ConnectionPool(std::shared_ptr<ThreadPool> pool)
    :m_pool(pool)
    ,m_zk(pool)
{
    m_zk.reConnect();
}
std::unique_ptr<TcpClient> ConnectionPool::getConnection(const std::string &servicename)
{
    return std::unique_ptr<TcpClient>();
}

void ConnectionPool::initZkNode()
{
    std::vector<std::string> res = m_zk.getNodeChildren("/rpc/service");
    for(auto& value : res)
    {
        std::cout << value << std::endl;
    }
}
