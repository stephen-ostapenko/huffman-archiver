#!/usr/bin/python3

from sys import argv
from random import randint

file = argv[1]
sz = int(argv[2])
mod = int(argv[3])

fo = open(file, 'wb')

for i in range(sz):
	c = randint(0, mod - 1)
	fo.write(bytes([c]))

fo.close()