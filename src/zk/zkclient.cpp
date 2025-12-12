#include "zkclient.h"

#include <string>
#include <iostream>
#include <time.h>
#include <semaphore>
#include "Logger.h"
// typedef void (*watcher_fn)(zhandle_t *zh, int type,
//         int state, const char *path,void *watcherCtx);
void watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE)
    {
        sem_t *sem = (sem_t *)(zoo_get_context(zh));
        sem_post(sem);
    }
}

ZkClient::ZkClient()
{
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
}

void ZkClient::start()
{
    std::string host = "192.168.105.2";
    std::string port = "2181";
    std::string hostaddr = host + ":" + port;

    m_handle = zookeeper_init(hostaddr.c_str(), watcher, 5000, 0, nullptr, 0);
    if (m_handle == nullptr)
    {
        std::cout << "zookeeper 初始化错误" << std::endl;
        return;
    }
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_handle, &sem);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    if (sem_timedwait(&sem, &ts) != 0)
    {
        if (errno == ETIMEDOUT)
        {
            LOG_ERROR("连接zookeeper超时");
        }
        else
        {
            LOG_ERROR("连接zookeeper失败");
        }
        sem_destroy(&sem);
        zookeeper_close(m_handle);
        m_handle = nullptr;
        LOG_FATAL("连接zookeeper失败,退出");
    }
    sem_destroy(&sem);
    LOG_DEBUG("连接zookeeper成功");
}
struct CallbackContext
{
    int rc;
    sem_t sem;
};

void exists_callback(int rc, const struct Stat *stat, const void *data)
{
    CallbackContext *ctx = (CallbackContext *)data;
    ctx->rc = rc;
    sem_post(&ctx->sem);
}
void createcomplete(int rc, const char *value, const void *data)
{
    CallbackContext *ctx = (CallbackContext *)data;
    ctx->rc = rc;
    sem_post(&ctx->sem);
}
void ZkClient::createNode(std::string path, std::string value)
{
    struct CallbackContext exist_ctx, create_ctx;
    sem_init(&exist_ctx.sem, 0, 0);
    zoo_aexists(m_handle, path.c_str(), 0, exists_callback, &exist_ctx);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    if (sem_timedwait(&exist_ctx.sem, &ts) != 0)
    {
        std::cout << "检查节点存在超时" << std::endl;
        sem_destroy(&exist_ctx.sem);
        return;
    }
    sem_destroy(&exist_ctx.sem);
    if (exist_ctx.rc != ZNONODE)
    {
        if (exist_ctx.rc == ZOK)
        {
            std::cout << "节点已存在" << std::endl;
        }
        else
        {
            std::cout << "你没有权限查看" << std::endl;
        }
        return;
    }
    sem_init(&create_ctx.sem, 0, 0);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    zoo_acreate(m_handle, path.c_str(), value.c_str(), value.length(), &ZOO_READ_ACL_UNSAFE, ZOO_EPHEMERAL, createcomplete, &create_ctx);
    if (sem_timedwait(&create_ctx.sem, &ts) != 0)
    {
        std::cout << "创建节点超时" << std::endl;
        sem_destroy(&create_ctx.sem);
        return;
    }
    std::cout << "怎么回事" << std::endl;
    sem_destroy(&create_ctx.sem);
    std::cout << create_ctx.rc << std::endl;
    if (create_ctx.rc == ZOK)
    {
        std::cout << "创建节点成功:" << path << std::endl;
    }
    else if (create_ctx.rc == ZCONNECTIONLOSS || create_ctx.rc == ZOPERATIONTIMEOUT)
    {
        std::cout << "连接断开或者超时" << std::endl;
    }
}

void ZkClient::getNode(std::string path)
{
    
}
