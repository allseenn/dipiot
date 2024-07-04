TARGET=web
ARGS=-lssl -lcrypto -pthread

all: $(TARGET)

$(TARGET): web.c
	gcc $(ARGS) -o $@ web.c

clean:
	rm -f $(TARGET)