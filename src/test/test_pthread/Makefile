CPPFLAGS     = 
CFLAGS       = -g
OUTPUT_OPT   = -o
LDFLAGS      = -pthread

MAKEFILE     = Makefile
PROGRAM      = a.out
ALL_HEADERS  = a.h common.h reception.h waiter.h
ALL_SOURCES  = a.c reception.c waiter.c
ALL_OBJECTS  = $(subst .c,.o, $(ALL_SOURCES))
RM_OPT       = --force --recursive

AR           = ar
RM           = rm
CTAGS        = ctags
PREFIX       = /usr/local


all: $(PROGRAM) tags

$(PROGRAM): $(ALL_OBJECTS)
	$(CC) $(OUTPUT_OPT) $@ $^ $(LDFLAGS)

tags: $(MAKEFILE) $(ALL_HEADERS) $(ALL_SOURCES)
	$(CTAGS) $^

.PHONY: all clean install

clean:
	$(RM) $(RM_OPT) $(ALL_OBJECTS) $(PROGRAM) tags core.*

install: $(PROGRAM)
	install $(PROGRAM) $(PREFIX)/bin/
	install rsyslog.d/$(PROGRAM).conf /etc/rsyslog.d/
	install logrotate.d/$(PROGRAM) /etc/logrotate.d/

###########################################################
# NOTE: DO NOT EDIT, autogen by `gcc -MM *.c'
###########################################################
a.o: a.c a.h common.h reception.h waiter.h
reception.o: reception.c reception.h
waiter.o: waiter.c waiter.h
