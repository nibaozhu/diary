#!/usr/bin/python

import py_compile

py_compile.compile('40.py')

supplies = ['pens', 'staplers', 'flame-throwers', 'binders']

for i in range(len(supplies)):
	print('Index ' + str(i) + ' in supplies is: ' + supplies[i])

