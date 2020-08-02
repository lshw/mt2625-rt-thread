#!/bin/bash
# debian8

if ! [ -x /usr/bin/arm-none-eabi-gcc ] ; then
apt-get install gcc-arm-none-eabi binutils-arm-none-eabi scons
fi

if ! [ "`which xz`" ] ; then
apt-get install xz-utils
fi

if ! [ -x libraries/mt2625 ] ; then
tar Jxf libraries/mt2625.tar.xz -C libraries
fi

cd project/nbiot_demo
scons -j 2

