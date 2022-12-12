CC = cc
CFLAGS = -Wall -Wpedantic
INCLUDES = ../src/

util: OBJS = test_util.c ../src/util.c
cm: OBJS = test_cm.c ../src/emon_CM.c
data: OBJS = test_data.c ../src/data.c ../src/util.c

all: util cm data

util:
	${CC} ${CFLAGS} -I${INCLUDES} -o $@.test $(OBJS) ${LIBS}
	./util.test
cm:
	${CC} ${CFLAGS} -I${INCLUDES} -o $@.test $(OBJS) ${LIBS}
	./cm.test
data:
	${CC} ${CFLAGS} -I${INCLUDES} -o $@.test $(OBJS) ${LIBS}
	./data.test
clean:
	rm *.test
