#include <iostream>
#include "zkclient.h"
#include <thread>
#include "threadpool.h"
#include <chrono>

int main()
{
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>();
    pool->start();
    ZkClient client(pool);
    client.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "hello world" << std::endl;
    client.createNode("/aaaa", "hello");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::string result;
    std::string path = "/aaaa";
    bool res = client.getNode(path, result);
    if (res)
    {
        std::cout << result << std::endl;
    }
    else
    {
        std::cout << "获取失败" << std::endl;
    }
    for (int i = 0; i < 10; ++i)
    {
        result = "";
        std::this_thread::sleep_for(std::chrono::seconds(100));
        res = client.getNode(path, result);
        if (res)
        {
            std::cout << result << std::endl;
        }
        else
        {
            std::cout << "获取失败" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}