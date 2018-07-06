#!/bin/sh

# 将Makefile.am放在工程目录下的Debug目录中；
# 此脚本必须放在工程根目录下；

function generate_src_list()
{
    echo "server_SOURCES=\\";
    for SOURCE_FILE in `find . -iname "*.cpp"  -or -iname "*.cc" | grep -v "GameField.cpp" | sort -r`
    do
        echo ".$SOURCE_FILE \\";
    done;
    echo "../public/mongo_db/GameField.cpp"
    echo
}

function generate_inc_list()
{
    echo "server_CPPFLAGS=\\";
    for PATH_NAME in `find . -iname "*.h" | sed 's/\/[A-Z_a-z0-9]*\.h$//g' | sort -u | grep -v "Debug\|^\.$\|\.pb\.h"`
    do
        echo "-I.$PATH_NAME \\";
    done;
    echo "-I.. \\"
    echo "-I../../libserver \\"
    echo "-I/usr/local/include/mongo"
}

echo "bindir=\${prefix}/Debug"
echo 
echo "bin_PROGRAMS = server"
echo 
cat def.mk
echo
echo
generate_inc_list
echo
generate_src_list
echo


