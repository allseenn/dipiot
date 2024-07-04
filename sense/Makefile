TARGET=sense

all: ${TARGET}

${TARGET}: ${TARGET}.o bme680.o
	gcc -o ${TARGET} ${TARGET}.o bme680.o

bme680.o: bme680.c
	gcc -o bme680.o -c bme680.c

${TARGET}.o: ${TARGET}.c
	gcc -o ${TARGET}.o -c ${TARGET}.c

clean:
	rm -rf *.o ${TARGET}

