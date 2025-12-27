#pragma once
#include <zookeeper/zookeeper.h>
#include <string>
#include <functional>
#include "threadpool.h"
struct CallbackContext;
namespace miniRpc
{
    class ZkClient
    {

    public:
        zhandle_t *m_handle{nullptr};
        sem_t m_sem;
        bool m_connected{false};
        ThreadPool* m_pool;
        using NodeUpdateCallBack = std::function<void(const std::string &)>;
        NodeUpdateCallBack m_NodeUpdateCallBack;
        std::string m_ip;
        int16_t m_port;
    public:
        ZkClient(ThreadPool* pool,const std::string& ip,int16_t port);
        ~ZkClient();
        void start();
        bool createNode(std::string path, std::string value, int mode);
        // bool getNode(const std::string& path,std::string& value);
        std::vector<std::string> getNodeChildren(const std::string &path, int watch = 0);
        void waitForConnection();
        bool setWatch(std::string path);
        void reConnect();
        void setNodeUpdateCallBack(const NodeUpdateCallBack &cb)
        {
            m_NodeUpdateCallBack = cb;
        }
        void updateNode(const std::string &path);

    private:
        bool waitForAdd(CallbackContext &ctx, int timeout_seconds = 5);
    };
}
