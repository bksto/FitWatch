DEVICE = atmega1284
PROGRAMMER = avrisp2
PORT = usb
FILENAME = bseto001_lab10
COMPILEO = avr-gcc -Os -mmcu=$(DEVICE)
COMPILE = avr-gcc -mmcu=$(DEVICE)
PART = m1284p
all: build upload clean

build:
	$(COMPILEO) -c $(FILENAME).c
	$(COMPILE) -o $(FILENAME).elf $(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(FILENAME).elf $(FILENAME).hex

upload:
	avrdude -P $(PORT) -p $(PART) -c $(PROGRAMMER) -e -U flash:w:$(FILENAME).hex -F 

clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
	rm $(FILENAME).hex
