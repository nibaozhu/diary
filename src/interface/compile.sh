if test `arch` == "x86_64"; then
make --always-make
else
echo 'Your arch is '`arch`', Need x86_64.'
fi
