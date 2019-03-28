#!/bin/sh
fileList=`find $1 -name "*.yuv"`

for file in $fileList;
	do
		./encode.sh ${file}
	done