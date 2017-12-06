CPPFLAGS     = 
CFLAGS       = -g
OUTPUT_OPT   = -o
LDFLAGS      = -pthread

MAKEFILE     = Makefile
PROGRAM      = dog
ALL_HEADERS  = dog.h common.h task.h
ALL_SOURCES  = dog.c task.c
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
dog.o: dog.c dog.h common.h task.h
task.o: task.c task.h
