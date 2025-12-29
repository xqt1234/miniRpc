#pragma once
#include <cstdint>
namespace miniRpc
{
    struct RpcMsgHeader
    {
        uint64_t reqId;
        uint32_t magic;
        uint32_t datalength;
    };
    static const uint32_t kMagicNumber = 0x55AA55AA; 
}
