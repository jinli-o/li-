#!/usr/bin/bash

echo "正在编译..."

# 静默编译，只显示错误信息
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ -f "server" ]
then 
    echo "编译成功！"
    echo "启动服务器..."
    echo ""
    #ulimit -n 1000000
    ./server
else
    echo "编译失败！"
fi