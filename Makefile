CC = gcc



all: myshell

myshell: myshell.o
	$(CC) myshell.c -o myshell
	
myshell.o: myshell.c
	$(CC) $(FLAGS) -c myshell.c


.PHONY: clean all

clean:
	rm -f *.o myshell
