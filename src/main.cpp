#include <iostream>
#include "zkclient.h"
#include <thread>
#include "threadpool.h"
int main()
{
    ZkClient client;
    client.start();
    client.createNode("/aaaa","hello");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}