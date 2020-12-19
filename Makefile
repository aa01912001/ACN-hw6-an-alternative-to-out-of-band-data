hw6_server.out, hw6_client.out: hw6_server.o hw6_client.o
	gcc -o hw6_server.out hw6_server.o
	gcc -o hw6_client.out hw6_client.o

hw6_server.o: hw6_server.c
	gcc -c hw6_server.c

hw6_client.o: hw6_client.c
	gcc -c hw6_client.c

clean:
	rm -f *.o *.out