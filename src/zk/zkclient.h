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
    zhandle_t* m_handle;
    sem_t m_sem;
    bool m_connected{false};
    std::shared_ptr<ThreadPool> m_pool;
public:
    ZkClient(std::shared_ptr<ThreadPool> pool);
    ~ZkClient();
    void start();
    bool createNode(std::string path,std::string value,int mode);
    bool getNode(const std::string& path,std::string& value);
    std::vector<std::string> getNodeChildren(const std::string &path);
    void waitForConnection();
    void reConnect();
private:
    bool waitForAdd(CallbackContext &ctx, int timeout_seconds = 5);
};
