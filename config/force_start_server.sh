#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Error: too few params"
    echo "please input: sh force_start_server.sh index"
    exit 1
fi

dir=`dirname $0`
cd $dir
url=`pwd`
cd $url
server_name=`echo ${url} | awk -F/ '{print $3}'`

echo -e "{\n\t\"op\" : \"start_server\",\n\t\"index\":$1\n}" > daemon.json
ps aux | grep machine | grep ${server_name} | grep "daemon/daemon" | grep -v grep
ps aux | grep machine | grep ${server_name} | grep "daemon/daemon" | grep -v grep | awk '{print "kill -34",$2}' | sh