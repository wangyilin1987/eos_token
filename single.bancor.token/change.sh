#!/bin/bash
basedir=`pwd`
echo $basedir
cd $basedir

echo $1

if [ $1 != "hot" -a $1 != "tic" -a $1 != "omo" -a $1 != "put" ];then
    echo $1
    echo "input error.eg:replace.sh [hot|tic|put f|d]."
    exit
fi

if [ $1 = "hot" ];then
    key_a="hot"
    key_b="HOT"
elif [ $1 = "tic" ] ; then
    key_a="tic"
    key_b="TIC"
elif [ $1 = "omo" ] ; then
    key_a="omo"
    key_b="OMO"
elif [ $1 = "put" ] ; then
    key_a="put"
    key_b="PUT"
else
    echo $1
    echo "input error.eg:replace.sh [hot|tic|omo|put f|d]."
    exit
fi

for file in `find ./ -type f -name "*[ch]pp" -print`
do
    currdir=`dirname $file`
    currname=`basename $file`
    cd $currdir
    newname=`echo $currname | sed "s/eos/$key_a/g"`
    echo "new:$newname"

    mv $currname $newname
    cd $basedir
done

find ./ ! -path "*/\.*" -type f -name "*[ch]pp" -print | xargs grep  eos  -l | xargs sed -i "s/eos/$key_a/g"
find ./ ! -path "*/\.*" -type f -name "*[ch]pp" -print | xargs grep  EOS  -l | xargs sed -i "s/EOS/$key_b/g"

echo "end"

