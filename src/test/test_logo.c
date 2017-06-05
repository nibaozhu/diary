#include <stdio.h>

#define S(X) #X
#define netframe \
"                                                             \n\
     ___          ___                                         \n\
    / _ \\  __    /__/                                        \n\
    || ||_ ||   |/    _         __  _  _                      \n\
    || |/_\\||_  ||__ /_\\   _   / _\\/_\\/_\\                \n\
    || ||_ ||_\\ ||_/|/   //_\\\\ || || ||_                   \n\
    || ||_\\||   ||  ||  ||  || || || ||_\\                   \n\
    || ||_ |\\___||  ||  \\\\  || || || ||_                   \n\
    \\| |\\_/||__/\\|  \\|   \\\\//\\\\|/ |/ |\\_/            \n\
"

int main(int argc, char** argv) {
    return printf("%s\n\n%s\n", netframe, S(netframe));
}
