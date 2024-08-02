TARGET=mqtt
ARGS=-lpaho-mqtt3c

all: $(TARGET)

$(TARGET): $(TARGET).c
	gcc $(ARGS) -o $@ $(TARGET).c

clean:
	rm -f $(TARGET)

install:
	chmod 777 $(TARGET) bsec.sh
	sudo cp $(TARGET) bsec.sh /usr/local/sbin
	
	

