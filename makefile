.SUFFIXES: .c .o .f .F

CC			=  mpicc  
CFLAGS			=  -g -Wall

FILES =  main.o aux.o hqsort.o
FILES2 =  mainStats.o aux.o hqsort.o

main.ex: $(FILES) mainStats.ex
	${CC} ${CFLAGS} -o main.ex -lm $(FILES) 

mainStats.ex: $(FILES2) 
	${CC} ${CFLAGS} -o mainStats.ex -lm $(FILES2)

.c.o:
	${CC} ${CFLAGS} $< -c -o $@

clean:
	rm *.o *.ex
