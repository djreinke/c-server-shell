#Makefile for remote server shell by Dan Reinke & Ryan Carl
both: client server 

client: client.o
	gcc -o client client.o

server: server.o ourshell.o
	gcc -o server ourshell.o server.o

client.o: client.c
	gcc -c client.c

server.o: server.c
	gcc -c server.c

ourshell.o: ourshell.c
	gcc -c ourshell.c

debug:
	gcc -g -c ourshell.c
	gcc -g -c server.c
	gcc -g -c client.c
	gcc -g -o server ourshell.o server.o
	gcc -g -o client client.o

clean:
	rm *.o server client

submit:
	tar -cvf lab11-djreinke-rpcarl.tar *.c Makefile 
