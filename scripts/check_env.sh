#!/usr/bin/env bash

ROOT=`pwd`

#检查环境变量
if [ -z "$NITE2_REDIST64" ];then
    echo "No NiTE2 env"
    exit 1
fi

#检查NiTE2目录

if [ -d "$NITE2_REDIST64/NiTE2" ];then
    ln -s $NITE2_REDIST64/NiTE2 $ROOT/NiTE2
else
    echo "No NiTE2 data"
    exit 1
fi

