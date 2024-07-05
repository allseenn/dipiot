TARGET=mqtt
ARGS=-lpaho-mqtt3c

all: $(TARGET)

$(TARGET): $(TARGET).c
	gcc $(ARGS) -o $@ $(TARGET).c

clean:
	rm -f $(TARGET)

