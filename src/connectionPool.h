#pragma once
#include "TcpClient.h"
#include <unordered_map>
#include "zkclient.h"
#include <thread>
#include <atomic>
#include "EventLoop.h"
#include <vector>
#include "TcpConnection.h"
using namespace mymuduo;
namespace miniRpc
{
    class ConnectionPool
    {
    private:
        std::unordered_map<std::string, std::vector<std::shared_ptr<TcpClient>>> m_activeClientMap;
        std::unordered_map<std::string, std::vector<std::shared_ptr<TcpClient>>> m_allClientMap;
        std::unordered_map<std::string, int> m_currentUse;
        std::shared_ptr<ThreadPool> m_pool;
        std::shared_ptr<ZkClient> m_zk;
        std::thread m_thread;
        std::atomic<bool> m_stop{false};
        TcpClient *m_client;
        std::function<void(const std::string &)> m_msgCallBack;

    public:
        ConnectionPool(std::shared_ptr<ThreadPool> pool, std::shared_ptr<ZkClient> zk);
        ~ConnectionPool();
        std::shared_ptr<TcpClient> getConnection(const std::string &servicename);
        void checkClients();
        void updateClients(const std::string &path);
        void setMessageCallBack(std::function<void(const std::string &)> cb);

    private:
        void startLoop();
        std::unique_ptr<mymuduo::EventLoop> m_loop;
        void updateClient(std::string servicename);
        void checkService();
        void createTcpClient(const std::string &ipPort, const std::string &clientName, const std::string &servicename);
        void sendHeart();
        void newConnection(const mymuduo::TcpConnectionPtr &conn);
        void onMessage(const TcpConnectionPtr &conn, Buffer *buffer);
    };
}
