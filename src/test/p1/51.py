from functools import reduce

def replace_char(s, oldChar, newChar ):
    return reduce(lambda s, char: s.replace(char, newChar), oldChar, s)

s = 'hello\\world/yes:are*you?excute"hi<ok>please|'
new_s = replace_char(s, '\\/:*?"<>|', '+')
print(new_s)
