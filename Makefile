CFLAGS += -ansi -pthread
LDFLAGS += -lm

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

ifdef DEBUG
	CFLAGS += -g -O0
else
	CFLAGS += -O2
endif

exe = synceta
objs = $(exe).o

$(exe) : $(objs)
	"$(CC)" $(CFLAGS) $(LDFLAGS) $(objs) -o "$(exe)"

.PHONY: clean install uninstall

clean:
	rm -fv $(objs) "$(exe)"

install: $(exe)
	cp -v "$(exe)" "$(BINDIR)/"

uninstall:
	rm -v "$(BINDIR)/$(exe)"
