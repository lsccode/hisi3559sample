#!/bin/sh
if [ "$#" -ne "1" ]; then
	echo "usage: $0 filename"
	exit 1
fi

./sample_venc 4 $1 fixqp 23 
sleep 3

./sample_venc 4 $1 fixqp 28 
sleep 3

./sample_venc 4 $1 fixqp 33 
sleep 3

./sample_venc 4 $1 fixqp 38 
sleep 3

./sample_venc 4 $1 fixqp 43 
sleep 3

./sample_venc 4 $1 vbr 8388608 
sleep 3

./sample_venc 4 $1 vbr 4194304 
sleep 3

./sample_venc 4 $1 vbr 2097152 
sleep 3

./sample_venc 4 $1 vbr 1048576 
sleep 3

./sample_venc 4 $1 vbr 524288 
sleep 3

./sample_venc 4 $1 cbr 8388608 
sleep 3

./sample_venc 4 $1 cbr 4194304 
sleep 3

./sample_venc 4 $1 cbr 2097152 
sleep 3

./sample_venc 4 $1 cbr 1048576 
sleep 3

./sample_venc 4 $1 cbr 524288 
sleep 3




