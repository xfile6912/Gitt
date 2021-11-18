parser : argv_parser.o main.o
	gcc -o parser argv_parser.o main.o

argv_parser.o : argv_parser.c argv_parser.h
	gcc -c -o argv_parser.o argv_parser.c
  
main.o : main.c
	gcc -c -o main.o main.c
  
clean : 
	rm *.o parser