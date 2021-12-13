gitt : gitt.o sha1.o
	gcc -o gitt gitt.o sha1.o
  
gitt.o : gitt.c
	gcc -c -o gitt.o gitt.c

sha1.o : sha1.c
	gcc -c -o sha1.o sha1.c

clean : 
	rm -rf *.o gitt .gitt