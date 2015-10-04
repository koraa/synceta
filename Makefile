CFLAGS += -ansi -pthread

ifdef DEBUG
	CC += -g -O0
else
	CC += -O2
endif

exe = synceta
objs = $(exe).o

$(exe) : $(objs)
	$(CC) $(CFLAGS) $(LDFLAGS) $(objs) -o $(exe)

.PHONY: clean

clean:
	rm -fv $(objs) $(exe)
