#!/usr/bin/python

cat = ['fat', 'rat', 12, 'loud']

i1 = None
try:
	i1 = cat.index(121)
except ValueError:
	print(' 12 not exists' + \
'ddddddddddddddddd')
print(i1)
