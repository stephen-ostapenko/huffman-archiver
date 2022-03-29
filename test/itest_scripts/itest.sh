#!/bin/bash

cd $(dirname $0)
pwd
echo

PATH_TO_BIN=../../b/hw_02
PATH_TO_DATA="../itest_data"
NTEST=$(ls $PATH_TO_DATA | wc -w)

function run {
	$PATH_TO_BIN -c -f $1 -o archive > compress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	$PATH_TO_BIN -u -f archive -o tmp_file > decompress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '1p' < compress_output) <(sed -n '2p' < decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '2p' < compress_output) <(sed -n '1p' < decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '3p' < compress_output) <(sed -n '3p' < decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff $1 tmp_file
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	echo $((i + 1))/$NTEST done
}

echo Integration testing started:
i=0
f=0

echo
ls -l ../itest_data
echo

for file in $(ls $PATH_TO_DATA)
do
	run $PATH_TO_DATA/$file

	i=$((i + 1))
done

rm archive tmp_file
rm compress_output decompress_output

if [ $f -eq 0 ]
then
	echo all tests passed!
else
	echo $f errors!!!
fi

echo
