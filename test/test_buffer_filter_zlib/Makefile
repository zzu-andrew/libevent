ROOTDIR =  ./../..



EXE = test_buffer_filter_zlib

SRCS += ${wildcard *.cpp}


LDFLAGS = -I$(ROOTDIR)/inc -I./
CFLAGS = -g

all:${EXE}

${EXE}:${SRCS}
	echo ${ROOTDIR}
	echo ${SRCS}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent -lz


clean:
	rm -rf ${EXE}
	rm -rf *.o

