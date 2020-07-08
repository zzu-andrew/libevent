ROOTDIR =  ./../..



EXE = test_conf

LDFLAGS = -I$(ROOTDIR)/inc 
CFLAGS = -g

all:${EXE}

${EXE}:test_conf.cpp
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent


clean:
	rm -rf ${EXE}
	rm -rf *.o

