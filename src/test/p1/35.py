#!/usr/bin/python


def collatz(number):
	if number % 2 == 0:
		number = number//2
	else:
		if number > 0:
			number = 3*number+1
		else:
			number = 3*number-1
	print(number)
	return number

def call_me(number):
	print(number)
	while number != 1 and number != 0 and number != -1:
		number = collatz(number)

call_me(int(input(">> ")))
