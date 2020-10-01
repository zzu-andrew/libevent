
all:firstlibevent

firstlibevent:first_libevent.cpp
	g++ $^ -o $@ -levent -g
	./$@
clean:
	rm -rf firstlibevent
	rm -rf *.o

