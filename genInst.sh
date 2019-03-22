#!/bin/sh

if [ "$#" -ge "1" ]; then
fileList=`find $1 -name "*.prototxt"`

for file in $fileList;
do
filePath=${file%/*}
fileName=${file##*/}
fileShortName=${fileName%.*}

echo "file full path  : $file"
echo "file path       : $filePath"
echo "file name       : $fileName"
echo "file short name : $fileShortName"

cp data/detection/caffemodel/conv_standard.cfg $filePath/${fileShortName}.cfg
sed -i "s#\(\[prototxt_file\]\)#\1 ${file}#g" $filePath/${fileShortName}.cfg
sed -i "s#\(\[caffemodel_file\]\)#\1 ${filePath}/${fileShortName}.caffemodel#g" $filePath/${fileShortName}.cfg
sed -i "s#\(\[instruction_name\]\)#\1 ${filePath}/${fileShortName}_nnie_inst#g" $filePath/${fileShortName}.cfg

if [ "$#" -eq "1" ]; then
sed -i "s#\(\[image_list\]\)#\1 ${filePath}/${fileShortName}.fea#g" $filePath/${fileShortName}.cfg
fi

if [ "$#" -eq "2" ]; then
sed -i "s#\(\[image_list\]\)#\1 ${filePath}/$2.fea#g" $filePath/${fileShortName}.cfg
fi

done

fi



