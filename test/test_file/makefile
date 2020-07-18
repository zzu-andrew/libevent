ROOTDIR =  ./../..



EXE = test_file

LDFLAGS = -I$(ROOTDIR)/inc 
CFLAGS = -g -DEVENT_DEBUG_LOGGING_ENABLED

all:${EXE}

${EXE}:${EXE}.cpp
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent


clean:
	rm -rf ${EXE}
	rm -rf *.o

