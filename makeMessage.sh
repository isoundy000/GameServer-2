#!/bin/bash
MY_PATH=$(cd `dirname $0`; pwd)

update_message_proto(){
		OUT_DIR="/tmp/.temp_message_proto"
		mkdir -p "$OUT_DIR" && 
		cd "$OUT_DIR" &&
		svn up  &&
		/bin/cp -f "$OUT_DIR/"*.proto "$MY_PATH/message_new_proto/"
}

update_message_proto

cd "$MY_PATH"
#/bin/cp message_proto/* message_new_proto/ -vf
cd message && sh generate_proto.sh

