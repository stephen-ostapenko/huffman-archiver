#!/bin/bash

cd $(dirname $0)
pwd
echo

PATH_TO_BIN=../../b/hw_02
NTEST=25

function run {
	./gen.py xx $1 $2

	valgrind --quiet --leak-check=full --show-leak-kinds=all $PATH_TO_BIN -c -f xx -o yy > vr_compress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	valgrind --quiet --leak-check=full --show-leak-kinds=all $PATH_TO_BIN -u -f yy -o zz > vr_decompress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '1p' < vr_compress_output) <(sed -n '2p' < vr_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '2p' < vr_compress_output) <(sed -n '1p' < vr_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '3p' < vr_compress_output) <(sed -n '3p' < vr_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff xx zz
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	echo $((i + 1))/$NTEST done
}

echo Random valgrind integration testing started:
f=0

for ((i = 0; i < 5; i++))
do
	run 512 256
done

for ((i = 5; i < 10; i++))
do
	run 512 2
done

for ((i = 10; i < 15; i++))
do
	run 512 1
done

for ((i = 15; i < 20; i++))
do
	run 512 24
done

for ((i = 20; i < 25; i++))
do
	run 512 10
done

rm xx yy zz
rm vr_compress_output vr_decompress_output

if [ $f -eq 0 ]
then
	echo all tests passed!
else
	echo $f errors!!!
fi

echo
