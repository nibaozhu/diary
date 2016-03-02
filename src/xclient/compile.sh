NEED_ARCH='x86_64'

if test `arch` == $NEED_ARCH; then
make --always-make
else
echo 'Your arch is '`arch`', Need '$NEED_ARCH'.'
fi
