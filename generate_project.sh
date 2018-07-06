#!/bin/sh

function process_path() 
{
    cur_path=`pwd`;
    root_path=$1;
    subList=`ls -l -t $root_path | awk '{if ($1~/^d/) print $9}'`;
    if [ -z "$subList" ]; then
        return;
    fi;

    for subname in $subList
    do 
        if [ "$subname"X == "Debug"X -o "$subname"X == "autom4te.cache"X -o "$subname"X == "m4"X ]; then
            continue;
        fi;
        echo "$subname=$subname filter=\"*.h *.cpp *.json *.py *.hpp *.proto *.lua\" {";
        sub_path="$root_path/$subname";
        process_path "$root_path/$subname";
        echo "}"
        root_path=$1;
    done;
}

if [ $# -lt 1 ]; then
    echo "error, command: sh $0 path"; 
    exit;
fi;

echo "Project=$1 CD=. filter=\"*.h *.cpp\" flags=S {"

process_path $1;

echo "}"

