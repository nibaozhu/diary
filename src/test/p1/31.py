#!/usr/bin/python

def spam():
	global eggs
	eggs = 'spam'
	print(eggs)

def bacon():
	eggs = 'bacon'
	print(eggs)

def ham():
	print(eggs)

eggs = 43
spam()
bacon()
ham()
print(eggs)
