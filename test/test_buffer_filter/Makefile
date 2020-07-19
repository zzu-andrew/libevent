ROOTDIR =  ./../..



EXE = test_buffer_filter

LDFLAGS = -I$(ROOTDIR)/inc 
CFLAGS = -g

all:${EXE}

${EXE}:${EXE}.cpp
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent


clean:
	rm -rf ${EXE}
	rm -rf *.o

