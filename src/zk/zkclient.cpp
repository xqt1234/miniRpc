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
    ZkClient *zk = (ZkClient *)(watcherCtx);
    std::cout << "type:" << type << " state:" << state << std::endl;
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_post(&zk->m_sem);
            zk->m_connected = true;
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            zk->m_connected = false;
            zk->start();
        }
        else if (state == ZOO_CONNECTING_STATE)
        {
            zk->m_connected = false;
        }
    }
    std::cout << "watcher 被调用 " << std::endl;
}
void custom_zookeeper_log(const char *message) {
    // 什么都不做，完全屏蔽日志
    // 或者只输出特定级别的日志
}
ZkClient::ZkClient(std::shared_ptr<ThreadPool> pool)
{
    m_pool = pool;
    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
}

ZkClient::~ZkClient()
{
    if (m_handle != nullptr)
    {
        zookeeper_close(m_handle);
        m_handle = nullptr;
    }
    sem_destroy(&m_sem);
}

void ZkClient::reConnect()
{
    std::cout << "正在连接zk..." << std::endl;
    if(m_handle != nullptr)
    {
        zookeeper_close(m_handle);
        m_handle = nullptr;
    }
    std::string host = "192.168.105.2";
    std::string port = "2181";
    std::string hostaddr = host + ":" + port;
    sem_init(&m_sem, 0, 0);
    m_handle = zookeeper_init(hostaddr.c_str(), watcher, 1000, 0, this, 0);
    zoo_set_log_callback(m_handle,custom_zookeeper_log);
    if (m_handle == nullptr)
    {
        std::cout << "zookeeper 初始化错误" << std::endl;
        return;
    }
    waitForConnection();
    std::cout << "线程号是" << std::this_thread::get_id() << std::endl;
}

void ZkClient::waitForConnection()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    if (sem_timedwait(&m_sem, &ts) != 0)
    {
        zookeeper_close(m_handle);
        m_handle = nullptr;
        LOG_ERROR("首次连接zookeeper时失败");
    }
}
void ZkClient::start()
{
    m_pool->addTask(std::bind(&ZkClient::reConnect,this));
}
struct CallbackContext
{
    int rc;
    sem_t sem;
    std::string value;
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
    if(m_handle == nullptr || m_connected == false)
    {
        std::cout << "已经离线，等待连接" << std::endl;
        return;
    }
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

void node_data_completion_t(int rc, const char *value, int value_len,
                            const struct Stat *stat, const void *data)
{
    auto *ctx = static_cast<CallbackContext *>(const_cast<void *>(data));
    ctx->rc = rc;
    if (rc == ZOK && value != nullptr)
    {
        ctx->value.assign(value, value_len);
    }
    sem_post(&ctx->sem);
}
bool ZkClient::getNode(const std::string &path, std::string &value)
{
    if(m_handle == nullptr || m_connected == false)
    {
        std::cout << "已经离线，等待连接" << std::endl;
        return false;
    }
    struct CallbackContext ctx;
    sem_init(&ctx.sem, 0, 0);
    zoo_aget(m_handle, path.c_str(), 0, node_data_completion_t, &ctx);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    if (sem_timedwait(&ctx.sem, &ts) != 0)
    {
        std::cout << "获取值失败" << std::endl;
        sem_destroy(&ctx.sem);
        return false;
    }
    sem_destroy(&ctx.sem);
    if (ctx.rc != ZOK)
    {
        return false;
    }
    value = ctx.value;
    return true;
}
struct GetChildrenCallbackContext
{
    sem_t sem;
    int rc;                                 // 操作结果码
    std::vector<std::string> childrenNames; // 存储解析后的子节点名称
    struct Stat nodeStat;
};
void getStrings_completion_t(int rc,
                             const struct String_vector *strings, const void *data)
{
    GetChildrenCallbackContext *ctx = (GetChildrenCallbackContext *)data;
    if (rc == ZOK && strings != nullptr)
    {
        ctx->childrenNames.clear();
        for (int i = 0; i < strings->count; ++i)
        {
            ctx->childrenNames.push_back(std::string(strings->data[i]));
        }
        deallocate_String_vector((struct String_vector *)strings);
    }
    else
    {
        std::cout << "获取节点失败" << std::endl;
    }
    sem_post(&ctx->sem);
}
std::vector<std::string> ZkClient::getNodeChildren(const std::string &path)
{
    std::vector<std::string> childrenNames;
    if(m_handle == nullptr || m_connected == false)
    {
        std::cout << "已经离线，等待连接" << std::endl;
        return childrenNames;
    }
    struct GetChildrenCallbackContext ctx;
    
    zoo_aget_children(m_handle, path.c_str(), 0, getStrings_completion_t, &ctx);
    sem_init(&ctx.sem, 0, 0);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    sem_timedwait(&ctx.sem, &ts);
    childrenNames.swap(ctx.childrenNames);
    return childrenNames;
}
