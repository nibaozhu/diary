#!/usr/bin/python

while True:
	password = None
	print('Who are you?')
	name = input()
	if name != 'Joe':
		continue
	while True:
		print('Hello, Joe. What is the password? (It is a fish.)')
		password = input()
		if password == 'swordfish':
			break
	if password:
		break
print('Access granted.')
