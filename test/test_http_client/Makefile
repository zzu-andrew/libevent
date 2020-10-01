ROOTDIR =  ./../..


SOURCE  += ${wildcard *.cpp}

EXE = $(basename ${SOURCE})

LDFLAGS = -I$(ROOTDIR)/inc 
CFLAGS = -g

all:${EXE} ${EXE2}

${EXE}:${SOURCE}
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent

${EXE2}:${SOURCE2}
	echo ${ROOTDIR}
	g++ ${LDFLAGS} ${CFLAGS} $^ -o $@ -levent

clean:
	rm -rf ${EXE}
	rm -rf *.o

