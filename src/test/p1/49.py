#!/usr/bin/python

birthdays = {'alice': 'Apr 1', 'Bob': 'Dec 12', 'Carol': 'Mar 4', 'Zhang Wenjuan': 'Oct 2'}

while True:
	print('Enter a name: (blank to quit)')
	name = input()
	if name == '':
		break

	if name in birthdays:
		print('\'' + birthdays[name] + '\' is the birthday of ' + name)
	else:
		print('i do not have birthday information for ' + name)
		print('what is their birthday?')
		bday = input()
		birthdays[name] = bday
		print('Birthday database updated')
