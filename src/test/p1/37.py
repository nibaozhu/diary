#!/usr/bin/python

import py_compile

spam = ['cat', 'bat', 'rat', 'elephant']
del spam[2]
print(spam)

py_compile.compile('37.py')
