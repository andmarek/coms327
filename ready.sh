#!/bin/bash

set -eu

read -p "assignment num: " assn

assn=assignment-$assn

if [ ! -f $assn/README ]; then
	printf "WARNING: README not found\n"
fi

if [ ! -f $assn/CHANGELOG ]; then
	printf "WARNING: CHANGELOG not found\n"
fi

if [ ! -f $assn/Makefile ]; then
	printf "WARNING: Makefile not found\n"
fi

read -p "firstname lastname: " first last

dirname="${last}_${first}.$assn"
dir="/tmp/$dirname"

cp -r $assn $dir

wd=`pwd`
cd /tmp && tar -czf $wd/$dirname.tar.gz $dirname

rm -rf $dir
