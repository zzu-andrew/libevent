ROOTDIR =  ./../..



EXE = test_http_server

LDFLAGS = -I$(ROOTDIR)/inc 
CFLAGS = -g
SRCS += ${wildcard *.cpp}

all:${EXE}

${EXE}:${SRCS}  libevent.a
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@


clean:
	rm -rf ${EXE}
	rm -rf *.o

