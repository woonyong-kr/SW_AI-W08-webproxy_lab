#!/bin/bash

#
# free-port.sh - 사용하지 않는 TCP 포트를 반환합니다.
#
PORT_START=4500
PORT_MAX=65000
port=${PORT_START}

while [ TRUE ] 
do
  portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip | grep tcp | \
    cut -c21- | cut -d':' -f2 | cut -d' ' -f1 | grep -E "[0-9]+" | uniq | tr "\n" " "`

  echo "${portsinuse}" | grep -wq "${port}"
  if [ "$?" == "0" ]; then
    if [ $port -eq ${PORT_MAX} ]
    then
      exit -1
    fi
    port=`expr ${port} + 1`
  else
    echo $port
    exit 0
  fi
done
