#pragma once
#include <zookeeper/zookeeper.h>
#include <string>
#include <functional>
#include "threadpool.h"
/*
所有的都用同步即可，
*/
struct CallbackContext;
class ZkClient
{

public:
    zhandle_t* m_handle{nullptr};
    sem_t m_sem;
    bool m_connected{false};
    std::shared_ptr<ThreadPool> m_pool;
    using NodeUpdateCallBack = std::function<void(const std::string&)>;
    NodeUpdateCallBack m_NodeUpdateCallBack;
public:
    ZkClient(std::shared_ptr<ThreadPool> pool);
    ~ZkClient();
    void start();
    bool createNode(std::string path,std::string value,int mode);
    // bool getNode(const std::string& path,std::string& value);
    std::vector<std::string> getNodeChildren(const std::string &path,int watch = 0);
    void waitForConnection();
    bool setWatch(std::string path);
    void reConnect();
    void setNodeUpdateCallBack(const NodeUpdateCallBack& cb)
    {
        m_NodeUpdateCallBack = cb;
    }
    void updateNode(const std::string& path);
private:
    bool waitForAdd(CallbackContext &ctx, int timeout_seconds = 5);
};
