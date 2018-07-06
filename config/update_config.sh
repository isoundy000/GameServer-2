#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Error: too few params"
    echo "please input: sh update_config.sh folder"
    exit 1
fi

dir=`dirname $0`
cd $dir
url=`pwd`
cd $url
server_name=`echo ${url} | awk -F/ '{print $3}'`

foler_name=`echo $1 | sed 's/\///g'`
ver_date=$(date +%Y.%m.%d.%H.%M.%S)

sub_value=0
if [ $# -ge 2 ]; then
	sub_value=`echo $2`
fi

echo -e "{\n\t\"op\" : \"update_config\",\n\t\"folder\" : \"$foler_name\",\n\t\"sub_value\" : $sub_value,\n\t\"tick\" : \"$ver_date\"\n}" > update_config.json

echo "updating config $1 ...";
ps aux | grep machine | grep ${server_name} | grep -v grep | grep -v daemon | awk '{print "kill -35",$2}' | sh
echo "updated config $1"