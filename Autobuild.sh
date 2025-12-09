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

if ls `pwd`/bin/log/*.log >/dev/null 2>&1;then
    rm  `pwd`/bin/log/*.log
fi

cd `pwd`/build && cmake .. && make 

cd ..