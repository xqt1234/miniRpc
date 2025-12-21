#include "rpcChannel.h"
#include "connectionPool.h"
#include "TcpClient.h"
using namespace mymuduo;
RpcChannel::RpcChannel()
{
    m_threadPool = std::make_shared<ThreadPool>();
    m_threadPool->start();
    m_client = std::make_shared<ZkClient>(m_threadPool);
    m_client->reConnect();
    m_connPool = std::make_shared<ConnectionPool>(m_threadPool,m_client);
}

void RpcChannel::callMethodAsync(const std::string &serviceName, 
    const std::string &methodName, const std::string &request,
     std::function<void(const std::string &)> callback)
{
    std::shared_ptr<TcpClient> client = m_connPool->getConnection(serviceName);
    client->connection()->send(request);
}
