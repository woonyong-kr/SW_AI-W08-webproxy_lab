CC = gcc
CFLAGS = -O0 -Wall -I . -g
# CFLAGS = -O2 -Wall -I . -g

# 이 플래그는 Linux 환경에서 Pthreads 라이브러리를 포함합니다.
# 다른 시스템에서는 다른 설정이 필요할 수 있습니다.
LIB = -lpthread

all: tiny cgi

tiny: tiny.c csapp.o
	$(CC) $(CFLAGS) -o tiny tiny.c csapp.o $(LIB)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.o tiny *~
	(cd cgi-bin; make clean)
