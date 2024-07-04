TARGET=sense

all: ${TARGET}

${TARGET}: ${TARGET}.o bme680.o
	gcc -o ${TARGET} ${TARGET}.o bme680.o

bme680.o: bme680.c
	gcc -o bme680.o -c bme680.c

${TARGET}.o: ${TARGET}.c
	gcc -o ${TARGET}.o -c ${TARGET}.c

install:
	chmod 777 ${TARGET}
	sudo cp ${TARGET} /usr/local/sbin
	
clean:
	rm -rf *.o ${TARGET}

