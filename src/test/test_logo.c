#include <stdio.h>

#define S(X) #X
#define netframe \
"\n\
                  ___\n\
           __    /__/\n\
      _  _ ||   |/    _        __  _  _\n\
    |/ \\/_\\||_  ||__ /_\\   _  / _\\/_\\/_\\\n\
    || ||_ ||_\\ ||_/|/   __\\\\ || || ||_\n\
    || ||_\\||   ||  || //  || || || ||_\\\n\
    || ||_ |\\___||  || \\\\  || || || ||_\n\
    \\| |\\_/||__/\\|  \\|  \\\\//\\\\|/ |/ |\\_/\n\
"

int main(int argc, char** argv) {
    return printf("%s\n\n\t\t\t\t%s\n", netframe, S(netframe));
}
