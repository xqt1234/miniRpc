#pragma once
#include <string>
#include "rpcService.h"
#include "provider.h"
#include "rpcChannel.h"
#include "rpcconfig.h"
namespace miniRpc
{
    class RpcApplication
    {
    private:
        static std::unique_ptr<ZkClient> m_zkClient;
        static std::unique_ptr<ThreadPool> m_threadPool;
        static std::unique_ptr<RpcConfig> m_rpcConfig;

    public:
        RpcApplication();
        ~RpcApplication() = default;
        void init();
        static ZkClient& getZkClient();
        static ThreadPool& getThreadPool();
        static RpcConfig& getRpcConfig();
    };
}
