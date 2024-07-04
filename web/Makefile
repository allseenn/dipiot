TARGET=web
ARGS=-pthread

all: $(TARGET)

$(TARGET): web.c
	gcc $(ARGS) -o $@ web.c

clean:
	rm -f $(TARGET)