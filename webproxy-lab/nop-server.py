#!/usr/bin/python3

# nop-server.py - 동시성 테스트에서 head-of-line blocking을 만들기 위해
#                 사용하는 서버입니다. 연결을 하나 받은 뒤에는
#                 끝없이 대기 상태를 유지합니다.
#
# 사용법: nop-server.py <port>
#
import socket
import sys

# INET, STREAM 소켓 생성
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('', int(sys.argv[1])))
serversocket.listen(5)

while 1:
  channel, details = serversocket.accept()
  while 1:
    continue
