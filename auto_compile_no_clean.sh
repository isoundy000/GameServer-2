#!/bin/sh

# 此脚本必须在工程根目录下执行

rm -rf autoscan.log aclocal.m4 config.* configure depcomp install-sh missing ltmain.sh m4 stamp-h1 autom4te.cache lib_log

svn update

if [ $# -gt 0 ];then
    if [ $1 == "clean" ]; then
        exit 0;
    fi;
fi;

if [ ! -d Debug ]; then
    mkdir Debug;
fi;

if [ ! -d m4 ]; then
    mkdir m4;
fi;

sh g_makefile.sh > Debug/makefile.am

aclocal -I m4
autoheader
libtoolize --force
automake --add-missing
autoconf

CC="distcc g++" CXX="distcc g++" ./configure --prefix=`pwd`
if [ $? -ne 0 ]; then
    echo "confiture error;";
    exit -1;
fi;

cd message
sh generate_proto.sh

cd ../Debug
make -j12 all;
if [ $? -ne 0 ]; then
    echo "make error;";
    exit -1;
fi;

cd ..
rm -rf autoscan.log aclocal.m4 config.* configure depcomp install-sh missing ltmain.sh m4 stamp-h1 autom4te.cache lib_log
cd
sh update_tjxm.sh update
