#!/bin/sh

if [ "$#" -ne "1" ]; then
    echo "usage: $0 directory"
    exit 1
fi

fileType='-name *.prototxt'

find $1 $fileType | xargs sed -i 's#input: "data"#input: "data"\ninput_shape {#g'    
find $1 $fileType | xargs sed -i 's#input_dim#dim#g'                              
find $1 $fileType | xargs sed -i 's#layer {#}\nlayer {#g'        