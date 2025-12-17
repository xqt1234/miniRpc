#include "zkclient.h"

#include <string>
#include <iostream>
#include <time.h>
#include <semaphore>
#include "Logger.h"
void watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    ZkClient *zk = (ZkClient *)(watcherCtx);
    std::cout << "type:" << type << " state:" << state << std::endl;
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            
            sem_post(&zk->m_sem);
            std::cout << "11111" << std::endl;
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            zk->start();
            std::cout << "2222" << std::endl;
        }
        else if (state == ZOO_CONNECTING_STATE)
        {
            std::cout << "33333" << std::endl;
        }
    }else if (type == ZOO_CHILD_EVENT)  // 子节点变化事件
    {
        std::cout << "------子节点变化: " << path << std::endl;
        zk->updateNode(path);
    }
    else if (type == ZOO_CHANGED_EVENT)  // 节点数据变化事件
    {
        std::cout << "--------节点数据变化: " << path << std::endl;
    }
    else if (type == ZOO_CREATED_EVENT)  // 节点创建事件
    {
        std::cout << "--------节点创建: " << path << std::endl;
    }
    else if (type == ZOO_DELETED_EVENT)  // 节点删除事件
    {
        std::cout << "--------节点删除: " << path << std::endl;
    }
    else if (type == ZOO_NOTWATCHING_EVENT)  // watcher被移除事件
    {
        std::cout << "---------watcher被移除: " << path << std::endl;
    }
    std::cout << "watcher 被调用 " << std::endl;
}
void custom_zookeeper_log(const char *message)
{
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
    m_connected = false;
    std::cout << "正在连接zk..." << std::endl;
    if (m_handle != nullptr)
    {
        zookeeper_close(m_handle);
        m_handle = nullptr;
    }
    std::string host = "192.168.105.2";
    std::string port = "2181";
    std::string hostaddr = host + ":" + port;
    sem_init(&m_sem, 0, 0);
    m_handle = zookeeper_init(hostaddr.c_str(), watcher, 1000, 0, this, 0);
    zoo_set_log_callback(m_handle, custom_zookeeper_log);
    if (m_handle == nullptr)
    {
        std::cout << "zookeeper 初始化错误" << std::endl;
        return;
    }
    waitForConnection();
    m_connected = true;
    std::cout << "线程号是" << std::this_thread::get_id() << std::endl;
}
struct CallbackContext
{
    int rc;
    sem_t sem;
    std::string value;
    bool completed;
    CallbackContext() : rc(-1), completed(false)
    {
        sem_init(&sem, 0, 0);
    }
    ~CallbackContext()
    {
        sem_destroy(&sem);
    }
};

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
        std::cout << "连接失败" << std::endl;
    }else
    {
        std::cout << "连接成功" << std::endl;
    }
}



bool ZkClient::waitForAdd(CallbackContext &ctx, int timeout_seconds)
{
    if(ctx.completed)
    {
        return true;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_seconds;
    if (sem_timedwait(&ctx.sem, &ts) != 0)
    {
        std::cout << "检查节点存在超时" << std::endl;
        return false;
    }
    return true;
}

void ZkClient::start()
{
    m_pool->addTask(std::bind(&ZkClient::reConnect, this));
}
void ZkClient::updateNode(const std::string& path)
{
    if(m_NodeUpdateCallBack)
    {
        m_pool->addTask(m_NodeUpdateCallBack,path);
    }
}

void exists_callback(int rc, const struct Stat *stat, const void *data)
{
    CallbackContext *ctx = (CallbackContext *)data;
    ctx->rc = rc;
    ctx->completed = true;
    sem_post(&ctx->sem);
}

bool ZkClient::setWatch(std::string path)
{
    if (m_handle == nullptr || m_connected == false)
    {
        return false;
    }
    struct CallbackContext exist_ctx;
    zoo_aexists(m_handle, path.c_str(), 1, exists_callback, &exist_ctx);
    if (!waitForAdd(exist_ctx))
    {
        return false;
    }
    return true;
}
void createcomplete(int rc, const char *value, const void *data)
{
    CallbackContext *ctx = (CallbackContext *)data;
    ctx->rc = rc;
    ctx->completed = true;
    sem_post(&ctx->sem);
}


bool ZkClient::createNode(std::string path, std::string value,int mode)
{
    if (m_handle == nullptr || m_connected == false)
    {
        if(m_handle == nullptr)
        {
            std::cout << "m_handle为空" << std::endl;
        }
        if(m_connected == false)
        {
            std::cout << "m_connected为false" << std::endl;
        }
        std::cout << "已经离线，等待连接" << std::endl;
        return false;
    }
    struct CallbackContext exist_ctx, create_ctx;
    zoo_aexists(m_handle, path.c_str(), 0, exists_callback, &exist_ctx);
    if (!waitForAdd(exist_ctx))
    {
        return false;
    }
    if (exist_ctx.rc != ZNONODE)
    {
        if (exist_ctx.rc == ZOK)
        {
            std::cout << "节点已存在" << std::endl;
            return true;
        }
        else
        {
            std::cout << "你没有权限查看" << std::endl;
        }
        std::cout << "---------" << std::endl;
        return false;
    }
    //ZOO_EPHEMERAL
    zoo_acreate(m_handle, path.c_str(), value.c_str(), value.length(), &ZOO_OPEN_ACL_UNSAFE, mode, createcomplete, &create_ctx);
    if (!waitForAdd(create_ctx))
    {
        return false;
    }
    std::cout << "怎么回事" << std::endl;
    std::cout << create_ctx.rc << std::endl;
    if (create_ctx.rc == ZOK)
    {
        std::cout << "创建节点成功:" << path << std::endl;
    }
    else if (create_ctx.rc == ZCONNECTIONLOSS || create_ctx.rc == ZOPERATIONTIMEOUT)
    {
        std::cout << "连接断开或者超时" << std::endl;
        return false;
    }
    return true;
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
// bool ZkClient::getNode(const std::string &path, std::string &value)
// {
//     if (m_handle == nullptr || m_connected == false)
//     {
//         std::cout << "已经离线，等待连接" << std::endl;
//         return false;
//     }
//     struct CallbackContext ctx;
//     zoo_aget(m_handle, path.c_str(), 0, node_data_completion_t, &ctx);
//     if (!waitForAdd(ctx))
//     {
//         std::cout << "获取失败" << std::endl;
//         return false;
//     }
//     if (ctx.rc != ZOK)
//     {
//         return false;
//     }
//     value = ctx.value;
//     return true;
// }
struct GetChildrenCallbackContext
{
    sem_t sem;
    int rc;                                 // 操作结果码
    std::vector<std::string> childrenNames; // 存储解析后的子节点名称
    struct Stat nodeStat;
    bool completed;
    GetChildrenCallbackContext() : rc(-1), completed(false)
    {
        sem_init(&sem, 0, 0);
    }
    ~GetChildrenCallbackContext()
    {
        sem_destroy(&sem);
    }
};
void getStrings_completion_t(int rc,
                             const struct String_vector *strings, const void *data)
{
    GetChildrenCallbackContext *ctx = (GetChildrenCallbackContext *)data;
    if (rc == ZOK && strings != nullptr)
    {
        ctx->childrenNames.clear();
        std::cout << "节点个数:" << strings->count << std::endl;
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



std::vector<std::string> ZkClient::getNodeChildren(const std::string &path,int watch)
{
    std::vector<std::string> childrenNames;
    if (m_handle == nullptr || m_connected == false)
    {
        std::cout << "已经离线，等待连接" << std::endl;
        return childrenNames;
    }
    struct GetChildrenCallbackContext ctx;
    zoo_aget_children(m_handle,path.c_str(),1,getStrings_completion_t,&ctx);

    sem_init(&ctx.sem, 0, 0);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    if(sem_timedwait(&ctx.sem, &ts) != 0)
    {
        std::cout << "查找子节点超时" << std::endl;
    }
    childrenNames.swap(ctx.childrenNames);
    return childrenNames;
}

    // ZOOAPI int zoo_awget_children(zhandle_t *zh, const char *path,
    //     watcher_fn watcher, void* watcherCtx,
    //     strings_completion_t completion, const void *data);
        //zoo_awget_children(m_handle, path.c_str(),watcher ,&ctx, getStrings_completion_t,this );