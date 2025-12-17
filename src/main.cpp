#include <iostream>
#include "zkclient.h"
#include <thread>
#include "threadpool.h"
#include <chrono>
#include "provider.h"
#include "rpcService.h"
#include "connectionPool.h"
#include <vector>
#include <ranges>
#include <string_view>
int main()
{
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>();
    pool->start();
    //ZkClient client(pool);
    std::shared_ptr<ZkClient> client = std::make_shared<ZkClient>(pool);
    client->reConnect();
    ProVider provider(pool,client);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::shared_ptr<RpcService> service = std::make_shared<RpcService>();
    
    
    
    service->setServiceName("UserService2");
    provider.AddService(service);

    ConnectionPool connPool(pool,client);
    connPool.initNode();
    std::this_thread::sleep_for(std::chrono::seconds(40));
    return 0;
}