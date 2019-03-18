#!/usr/bin/python3

import sys

print('name')
name = input()

print('age')
age = int(input())

if name == 'Alice':
	print('Hi, ' + name)
elif age < 12:
	print('You are not Alice, kiddo')
elif age > 200:
	print('Unlike you, Alice is not an undead, immortal vampire.')
elif age > 100:
	print('You are not Alice, grannie.')
else:
	print('Hello, stranger ' + name)

