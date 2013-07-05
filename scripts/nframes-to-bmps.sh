#! /bin/bash

if [ "$#" -ne 4 ]; then
echo "wrong number of arguments"
echo "usage: nframes_to_bmps nframe_files_base_path bmp_files_base_path first_number last_number"
exit 0
fi

nframe_files_base_path=$1
bmp_files_base_path=$2

i=$3
while [[ $i -le $4 ]]
do
	nframe_file_path=$nframe_files_base_path$i.nframe
	bmp_file_path=$bmp_files_base_path$i.bmp
	nframe-to-bmp $nframe_file_path $bmp_file_path
	(( i++ ))
done
