all: master bin_adder

master: master.o
	gcc master.o -o master -lm

bin_adder: bin_adder.o
	gcc bin_adder.o -o bin_adder -lm

master.o: master.c shared.h 
	gcc -c master.c

bin_adder.o: bin_adder.c
	gcc -c bin_adder.c

clean:
	rm *.o master bin_adder
