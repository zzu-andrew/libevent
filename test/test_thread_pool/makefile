ROOTDIR =  ./../..



EXE = test_thread_pool

SRCS += ${wildcard *.cpp}

LDFLAGS = -I$(ROOTDIR)/inc  
CFLAGS = -g 

all:${EXE}

${EXE}:${SRCS}
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent -lpthread


clean:
	rm -rf ${EXE}
	rm -rf *.o


