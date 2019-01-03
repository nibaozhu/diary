#!/usr/bin/python

import random

secretNumber = random.randint(1,20)
print('i am thinking of a number between 1 and 20')

for guessToken in range(0, 4):
	print('take a guess')
	guess = int(input())

	if guess < secretNumber:
		print('Your guess is lower')
	elif guess > secretNumber:
		print('Your guess is bigger')
	else:
		break

if guess == secretNumber:
	print('Good job! you guess my number in ' + str(guessToken) + ' guess')
else:
	print('Nope. The number I was thinking of was ' + str(secretNumber))
