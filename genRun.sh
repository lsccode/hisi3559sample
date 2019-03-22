#!/bin/sh

if [ "$#" -lt "1" ]; then
	echo "usage: $0 directory [featuremap]"
	exit 1
fi
./genInst.sh $*
./genwk.sh $*
