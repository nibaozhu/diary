CPPFLAGS     = -U DEBUG_LOGGING
CFLAGS       = -O0 -g
OUTPUT_OPT   = -o
LDFLAGS      = -pthread

MAKEFILE     = Makefile
PROGRAM      = a.out
ALL_HEADERS  = a.h common.h logging.h reception.h waiter.h
ALL_SOURCES  = a.c logging.c reception.c waiter.c
ALL_OBJECTS  = $(subst .c,.o, $(ALL_SOURCES))
RM_OPT       = --force --recursive

AR           = ar
RM           = rm
CTAGS        = ctags


all: $(PROGRAM) tags

$(PROGRAM): $(ALL_OBJECTS)
	$(CC) $(OUTPUT_OPT) $@ $^ $(LDFLAGS)

tags: $(MAKEFILE) $(ALL_HEADERS) $(ALL_SOURCES)
	$(CTAGS) $^

.PHONY: all clean

clean:
	$(RM) $(RM_OPT) $(ALL_OBJECTS) $(PROGRAM)

###########################################################
# NOTE: DO NOT EDIT, autogen by `gcc -MM *.c'
###########################################################
a.o: a.c a.h common.h logging.h reception.h waiter.h
logging.o: logging.c logging.h
reception.o: reception.c reception.h logging.h
waiter.o: waiter.c waiter.h logging.h
