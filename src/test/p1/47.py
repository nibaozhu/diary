#!/usr/bin/python

spam = ['apples', 'bananas', 'tofu', 'cats']

for i in range(len(spam) - 1):
	print(spam[i], end = ', ')

print('and', spam[-1])
