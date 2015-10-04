CFLAGS += -ansi -pthread
LDFLAGS += -lm

ifdef DEBUG
	CFLAGS += -g -O0
else
	CFLAGS += -O2
endif

exe = synceta
objs = $(exe).o

$(exe) : $(objs)
	$(CC) $(CFLAGS) $(LDFLAGS) $(objs) -o $(exe)

.PHONY: clean

clean:
	rm -fv $(objs) $(exe)
