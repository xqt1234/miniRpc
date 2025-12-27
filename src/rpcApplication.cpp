#include "rpcApplication.h"
using namespace miniRpc;
std::unique_ptr<ThreadPool> RpcApplication::m_threadPool = nullptr;
std::unique_ptr<ZkClient> RpcApplication::m_zkClient = nullptr;
std::unique_ptr<RpcConfig> RpcApplication::m_rpcConfig = nullptr;
RpcApplication::RpcApplication()
{
}

void miniRpc::RpcApplication::init()
{
    m_rpcConfig = std::make_unique<RpcConfig>();
    m_rpcConfig->loadConfig("rpc.ini");
    m_threadPool = std::make_unique<ThreadPool>();
    m_threadPool->start();
    std::string zkip = m_rpcConfig->getValue("zkip");
    int16_t zkport = atoi(m_rpcConfig->getValue("zkport").c_str());
    std::cout << "zkip: " << zkip << " zkport: " << m_rpcConfig->getValue("zkport") << std::endl;
    m_zkClient = std::make_unique<ZkClient>(m_threadPool.get(),zkip,zkport);
    m_zkClient->reConnect();
}

ZkClient &RpcApplication::getZkClient()
{
    return *m_zkClient.get();
}

ThreadPool &RpcApplication::getThreadPool()
{
    return *m_threadPool.get();
}

RpcConfig &miniRpc::RpcApplication::getRpcConfig()
{
    return *m_rpcConfig.get();
}
