#include <iostream>
#include "provider.h"

int main()
{
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>();
    pool->start();
    std::shared_ptr<ZkClient> client = std::make_shared<ZkClient>(pool);
    client->reConnect();
    ProVider provider(pool,client);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::shared_ptr<RpcService> service = std::make_shared<RpcService>();
    service->setServiceName("UserService2");
    provider.AddService(service);
    std::cout << "你好呀" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(300));
    return 0;
}