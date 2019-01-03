#!/usr/bin/python

name = None
while not name:
	print('Enter your name:')
	name = input()
	print('How many guests will you have?')
	numberOfGuests = int(input())
	if numberOfGuests:
		print('Be sure to have enough room for all your guests')
		print('Done')
