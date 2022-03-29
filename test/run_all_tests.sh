#!/bin/bash

echo Testing started:
echo

cd $(dirname $0)
pwd
echo

cd ../
mkdir b
cd b
if [ $? -ne 0 ]
then
	exit 4
fi
echo

cmake ..
if [ $? -ne 0 ]
then
	exit 3
fi
echo

make
if [ $? -ne 0 ]
then
	exit 2
fi
echo

echo Valgrind unit-testing:
valgrind --leak-check=full --show-leak-kinds=all ./test_hw_02
if [ $? -ne 0 ]
then
	exit 1
fi
echo

cd ../test/itest_scripts
if [ $? -ne 0 ]
then
	exit 4
fi

./vritest.sh
if [ $? -ne 0 ]
then
	exit 1
fi

./itest.sh
if [ $? -ne 0 ]
then
	exit 1
fi

./ritest.sh
if [ $? -ne 0 ]
then
	exit 1
fi

echo Testing done successfully!!!
echo
