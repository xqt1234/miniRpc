#!/bin/bash

set -e
set -u

if [ ! -d `pwd`/build ];then
    mkdir `pwd`/build
else
    rm -rf `pwd`/build/*
fi

# if [ ! -d `pwd`/bin ];then
#     mkdir `pwd`/bin
#     cp `pwd`/src/server/*.ini `pwd`/bin/
#     cp `pwd`/src/server/*.sql `pwd`/bin/
#     cp `pwd`/src/client/*.ini `pwd`/bin/
# fi

if [ ! -d `pwd`/ThirdParty/minirpc ];then
    mkdir -p `pwd`/ThirdParty/minirpc
    mkdir `pwd`/ThirdParty/minirpc/include
    mkdir `pwd`/ThirdParty/minirpc/lib
    cp `pwd`/src/*.h `pwd`/ThirdParty/minirpc/include/
    cp `pwd`/src/zk/*.h `pwd`/ThirdParty/minirpc/include/
    cp `pwd`/lib/*.so `pwd`/ThirdParty/minirpc/lib/
fi

if ls `pwd`/bin/log/*.log >/dev/null 2>&1;then
    rm  `pwd`/bin/log/*.log
fi

cd `pwd`/build && cmake .. && make 

cd ..