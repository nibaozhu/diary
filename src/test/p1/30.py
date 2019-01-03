#!/usr/bin/python

eggs = 333
def spam():
	global eggs
	eggs = 2111
	print(eggs)
spam()
print(eggs)
