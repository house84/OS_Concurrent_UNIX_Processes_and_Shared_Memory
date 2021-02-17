all: master branch

master: master.o
	gcc master.o -o master -lm

branch: branch.o
	gcc branch.o -o branch -lm

master.o: master.c master.h 
	gcc -c master.c

branch.o: branch.c
	gcc -c branch.c

clean:
	rm *.o master branch
