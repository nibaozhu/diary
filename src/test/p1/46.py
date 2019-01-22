#!/usr/bin/python

import copy

a = [1,2,3,4]
b = copy.deepcopy(a)
b[3] = '1'

print(a)
print(b[1:2])
