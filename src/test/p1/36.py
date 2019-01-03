#!/usr/bin/python

v1 = ['cat', 'bat', 'rat', 'elephant']
#  print(v1)

v2 = ['hello', 3.1415926, True, None, 43]
# print(v2)

# print(v2[0:3])

# for i in range(-1, -len(v2)-1, -1):
def printList(v2):
	for i in range(len(v2)):
		print(v2[i],end='|')
	print('')

printList(v2)
v2[3] = False
printList(v2)

# v3 = []
# print(v3)
