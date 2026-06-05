#!/usr/bin/bash

make clean

make client

if [ -f "client" ]
then 
    #ulimit -n 1000000
    ./client
else
    echo "没有可执行程序"
fi