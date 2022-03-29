#!/bin/bash

cd $(dirname $0)
pwd
echo

PATH_TO_BIN=../../b/hw_02
NTEST=500

function run {
	./gen.py x $1 $2

	$PATH_TO_BIN -c -f x -o y > r_compress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	$PATH_TO_BIN -u -f y -o z > r_decompress_output
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '1p' < r_compress_output) <(sed -n '2p' < r_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '2p' < r_compress_output) <(sed -n '1p' < r_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff <(sed -n '3p' < r_compress_output) <(sed -n '3p' < r_decompress_output)
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	diff x z
	if [ $? -ne 0 ]
	then
		f=$((f + 1))
		echo $((i + 1)) failed!!!
		return
	fi

	echo $((i + 1))/$NTEST done
}

echo Random integration testing started:
f=0

for ((i = 0; i < 100; i++))
do
	run 5000 256
done

for ((i = 100; i < 200; i++))
do
	run 5000 2
done

for ((i = 200; i < 300; i++))
do
	run 5000 1
done

for ((i = 300; i < 400; i++))
do
	run 5000 24
done

for ((i = 400; i < 500; i++))
do
	run 5000 10
done

rm x y z
rm r_compress_output r_decompress_output

if [ $f -eq 0 ]
then
	echo all tests passed!
else
	echo $f errors!!!
fi

echo
