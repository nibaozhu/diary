#!/usr/bin/python

def eggs(someParameter):
	someParameter.append('hello')
	someParameter.insert(2, 'world')

spam = [1,2,3]

print(spam)
eggs(spam)
print(spam)
