# 프록시 랩용 Makefile
#
# `handin` 규칙을 제외하면 이 파일은 자유롭게 수정해도 됩니다.
# 교수자나 조교는 여러분의 Makefile에서 `make`를 실행해 프록시를
# 소스에서 빌드합니다.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: proxy

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c csapp.h
	$(CC) $(CFLAGS) -c proxy.c

proxy: proxy.o csapp.o
	$(CC) $(CFLAGS) proxy.o csapp.o -o proxy $(LDFLAGS)

# 제출할 수 있는 tar 파일을 ../proxylab-handin.tar에 생성합니다.
# 이 규칙은 수정하지 마세요.
handin:
	(make clean; cd ..; tar cvf $(USER)-proxylab-handin.tar proxylab-handout --exclude tiny --exclude nop-server.py --exclude proxy --exclude driver.sh --exclude port-for-user.pl --exclude free-port.sh --exclude ".*")

clean:
	rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
