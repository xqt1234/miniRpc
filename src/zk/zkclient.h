#pragma once
#include <zookeeper/zookeeper.h>
#include <string>
class ZkClient
{
private:
    zhandle_t* m_handle;
public:
    ZkClient(/* args */);
    ~ZkClient() = default;
    void start();
    void createNode(std::string path,std::string value);
    void getNode(std::string path);
};
