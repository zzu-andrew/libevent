ROOTDIR =  ./../..


SOURCE2 = test_et_event_server.cpp
SOURCE = test_event_server.cpp

EXE = $(basename ${SOURCE})

EXE2 = $(basename ${SOURCE2})

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

