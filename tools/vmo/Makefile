.PHONY: all

all: vmo

packet.o: ../../main/packet.c
	$(CC) -g -c -o $@ $< -I ../../include

vmo: main.c packet.o
	$(CC) -g -o $@ $^ -I ../../main

all: 
