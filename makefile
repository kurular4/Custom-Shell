all: odev

odev: main.c
	gcc -o main main.c options.c
