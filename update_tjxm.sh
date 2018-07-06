#!/bin/sh

RSA_KEY=~/.ssh/id_rsa
SERVER_IP=192.168.18.234
SERVER_PATH=/data/jianyu_dev_s001a/server
CONFIG_SVN_PATH=svn://192.168.16.237:9052/jianyu/planning/配置文档/服务端配置文件/config

LOCAL_PATH=$(pwd)
LOCAL_PROGRAM=$LOCAL_PATH/Debug/server
LOCAL_PACK_PATH=$LOCAL_PATH/../realease

DATE_VAL=$(date +%Y.%m.%d.%H.%M)
TARGET_FILE_NAME="server_$DATE_VAL.tar.gz"

function usage()
{
    echo "help:"
    echo "1. sh update_163.sh send file_path"
    echo "       example file_path is line 35: /home/glendy/develop/server_local/server/Debug/server"
    echo "2. sh update_163.sh update"
    echo "      update program and config"
    echo "end."
}

function send_file()
{
    FILE_NAME=$1

    echo "sending file[$FILE_NAME] to 'root@$SERVER_IP:$SERVER_PATH'"

    scp -i $RSA_KEY $FILE_NAME root@$SERVER_IP:$SERVER_PATH
}

function decompress_program()
{
echo "decompressing remote server[$SERVER_IP] file..."

ssh -i $RSA_KEY root@$SERVER_IP << EOF
cd $SERVER_PATH
/bin/rm -rf server config
tar -zxf \`ls -t *.tar.gz | head -1\`;
find . -iname ".svn" | awk '{print "/bin/rm -rf",\$0}' | sh
EOF
}

function checkout_config()
{
    v_config_path=$LOCAL_PACK_PATH/config
    if [ -d $v_config_path ];then
        echo "updating config: $v_config_path"
        sleep 2
        cd $v_config_path
        svn update
    else
        echo "checkout config: $v_config_path, from: $CONFIG_SVN_PATH"
        sleep 2
        cd $LOCAL_PACK_PATH
        svn checkout $CONFIG_SVN_PATH config
    fi
}

function compress_program_and_config()
{
    echo "packing file[ $TARGET_FILE_NAME ], from[ $LOCAL_PACK_PATH ]"

    cd $LOCAL_PACK_PATH
    tar -zcf $TARGET_FILE_NAME server config
}

function update_program()
{
    echo "coping file[server] from [$LOCAL_PROGRAM]"
    mkdir -p $LOCAL_PACK_PATH
    cd $LOCAL_PACK_PATH
    /bin/rm -rf server config
    /bin/cp -rf $LOCAL_PROGRAM ./server

    checkout_config
    compress_program_and_config
    send_file $LOCAL_PACK_PATH/$TARGET_FILE_NAME
    decompress_program

    #/bin/rm -rf $LOCAL_PACK_PATH/server $LOCAL_PACK_PATH/*.tar.gz
}

function update_task_config()
{
    echo "coping file[server] from [$LOCAL_PROGRAM]"
    mkdir -p $LOCAL_PACK_PATH
    cd $LOCAL_PACK_PATH

    checkout_config
    scp -i $RSA_KEY ./config/ root@$SERVER_IP:$SERVER_PATH
    
}

TYPE=$1
FILE_NAME=$2

case $TYPE in
    send)
        send_file $FILE_NAME;;
    update)
        update_program;;
    task)
        update_task_config;;
    *)
        usage;;
esac

