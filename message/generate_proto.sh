#!/bin/sh

FILE_HEAD=`find ../message_new_proto -iname "*.proto" | sort | sed 's/..\/message_new_proto/./g' | sed 's/.proto/.pb.h \\\\/g'`
echo "FILE_HEAD=$FILE_HEAD" > def.mk
echo "" >> def.mk

make;
