TARGET=bsec
ARG=-lm -lrt -static -std=c99 -pedantic

all: ${TARGET}

${TARGET}: ${TARGET}.o bsec_integration.o bme680.o libalgobsec.a
	gcc -o ${TARGET} ${TARGET}.o bsec_integration.o bme680.o libalgobsec.a ${ARG}
	
bsec_integration.o:
	gcc -o bsec_integration.o -c bsec_integration.c
	
bme680.o:
	gcc -o bme680.o -c bme680.c

${TARGET}.o: ${TARGET}.c
	gcc -o ${TARGET}.o -c ${TARGET}.c

install:
	chmod 777 ${TARGET}
	sudo cp ${TARGET} bsec_iaq.csv bsec_iaq.config bsec_iaq.state /usr/local/sbin
	
clean:
	rm -rf *.o ${TARGET}

