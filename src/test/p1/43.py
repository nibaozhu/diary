#!/usr/bin/python

import random

messages = [
'it is decidedly so',
'yes definitely',
'reply hazy try again',
'ask again later',
'concentrate and ask again',
'my reply is no',
'outlook not so good',
'very doubtful'
]

for i in range(len(messages)):
	print(messages[random.randint(0, len(messages) - 1)])
