
all:firstlibevent

firstlibevent:first_libevent.cpp
	g++ $^ -o $@ -levent
	./$@
clean:
	rm -rf firstlibevent
	rm -rf *.o

