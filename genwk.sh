#!/bin/sh
if [ "$#" -lt "1" ]; then
	echo "usage: $0 XXX.cfg xxx.wk"
	exit 1
fi

fileList=`find $1 -name "*.cfg"`

for file in $fileList;
do
filePath=${file%/*}
fileName=${file##*/}
fileShortName=${fileName%.*}

TART_DIR=~/ipc/Hi3559AV100/mpp/sample/svp/multi-core/nnie/data/nnie_model/detection/
nnie_mapper $file
cp ${filePath}/${fileShortName}_nnie_inst.wk $TART_DIR
done


