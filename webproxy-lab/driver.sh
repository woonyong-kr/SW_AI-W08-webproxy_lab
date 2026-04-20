#!/bin/bash
#
# driver.sh - 프록시 랩용 간단한 자동 채점 스크립트입니다. 코드가
#     동시성 캐시 프록시처럼 동작하는지 확인하는 기본 점검을 수행합니다.
#
#     작성: David O'Hallaron, Carnegie Mellon University
#     갱신: 2/8/2016
# 
#     사용법: ./driver.sh
# 

# 점수 배점
MAX_BASIC=40
MAX_CONCURRENCY=15
MAX_CACHE=15

# 각종 상수
HOME_DIR=`pwd`
PROXY_DIR="./.proxy"
NOPROXY_DIR="./.noproxy"
TIMEOUT=5
MAX_RAND=63000
PORT_START=1024
PORT_MAX=65000
MAX_PORT_TRIES=10

# 기본 테스트에 사용할 텍스트 및 바이너리 파일 목록
BASIC_LIST="home.html
            csapp.c
            tiny.c
            godzilla.jpg
            tiny"

# 캐시 테스트에 사용할 텍스트 파일 목록
CACHE_LIST="tiny.c
            home.html
            csapp.c"

# 여러 테스트에서 가져올 파일
FETCH_FILE="home.html"

#####
# 보조 함수
#

#
# download_proxy - 원본 서버의 파일을 프록시를 통해 내려받습니다.
# 사용법: download_proxy <testdir> <filename> <origin_url> <proxy_url>
#
function download_proxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --proxy $4 --output $2 $3
    (( $? == 28 )) && echo "오류: ${TIMEOUT}초 안에 파일을 가져오지 못했습니다."
    cd $HOME_DIR
}

#
# download_noproxy - 원본 서버에서 파일을 직접 내려받습니다.
# 사용법: download_noproxy <testdir> <filename> <origin_url>
#
function download_noproxy {
    cd $1
    curl --max-time ${TIMEOUT} --silent --output $2 $3 
    (( $? == 28 )) && echo "오류: ${TIMEOUT}초 안에 파일을 가져오지 못했습니다."
    cd $HOME_DIR
}

#
# clear_dirs - 내려받기 디렉터리를 비웁니다.
#
function clear_dirs {
    rm -rf ${PROXY_DIR}/*
    rm -rf ${NOPROXY_DIR}/*
}

#
# wait_for_port_use - 인자로 받은 TCP 포트 번호가 실제로 사용될 때까지
#     반복 확인합니다. 5초 뒤에는 시간 초과로 처리합니다.
#
function wait_for_port_use() {
    timeout_count="0"
    portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
        | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
        | grep -E "[0-9]+" | uniq | tr "\n" " "`

    echo "${portsinuse}" | grep -wq "${1}"
    while [ "$?" != "0" ]
    do
        timeout_count=`expr ${timeout_count} + 1`
        if [ "${timeout_count}" == "${MAX_PORT_TRIES}" ]; then
            kill -ALRM $$
        fi

        sleep 1
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`
        echo "${portsinuse}" | grep -wq "${1}"
    done
}


#
# free_port - 사용 가능한 빈 TCP 포트를 반환합니다.
#
function free_port {
    # [PORT_START, PORT_START+MAX_RAND] 범위에서 임의의 포트를 생성합니다.
    # 같은 머신에서 여러 학생이 동시에 드라이버를 실행할 때 충돌을
    # 피하기 위해 필요합니다.
    port=$((( RANDOM % ${MAX_RAND}) + ${PORT_START}))

    while [ TRUE ] 
    do
        portsinuse=`netstat --numeric-ports --numeric-hosts -a --protocol=tcpip \
            | grep tcp | cut -c21- | cut -d':' -f2 | cut -d' ' -f1 \
            | grep -E "[0-9]+" | uniq | tr "\n" " "`

        echo "${portsinuse}" | grep -wq "${port}"
        if [ "$?" == "0" ]; then
            if [ $port -eq ${PORT_MAX} ]
            then
                echo "-1"
                return
            fi
            port=`expr ${port} + 1`
        else
            echo "${port}"
            return
        fi
    done
}


#######
# 메인 로직
#######

######
# 필요한 파일이 모두 있고 권한도 올바른지 확인합니다.
#

# 현재 사용자가 띄운 남아 있는 proxy 또는 tiny 서버를 종료합니다.
killall -q proxy tiny nop-server.py 2> /dev/null

# Tiny 디렉터리가 있는지 확인합니다.
if [ ! -d ./tiny ]
then 
    echo "오류: ./tiny 디렉터리를 찾을 수 없습니다."
    exit
fi

# Tiny 실행 파일이 없으면 빌드를 시도합니다.
if [ ! -x ./tiny/tiny ]
then 
    echo "tiny 실행 파일을 빌드합니다."
    (cd ./tiny; make)
    echo ""
fi

# 필요한 Tiny 파일이 모두 있는지 확인합니다.
if [ ! -x ./tiny/tiny ]
then 
    echo "오류: ./tiny/tiny 파일이 없거나 실행 가능한 파일이 아닙니다."
    exit
fi
for file in ${BASIC_LIST}
do
    if [ ! -e ./tiny/${file} ]
    then
        echo "오류: ./tiny/${file} 파일을 찾을 수 없습니다."
        exit
    fi
done

# 실행 가능한 proxy 파일이 있는지 확인합니다.
if [ ! -x ./proxy ]
then 
    echo "오류: ./proxy 파일이 없거나 실행 가능한 파일이 아닙니다. proxy를 다시 빌드한 뒤 재시도하세요."
    exit
fi

# 실행 가능한 nop-server.py 파일이 있는지 확인합니다.
if [ ! -x ./nop-server.py ]
then 
    echo "오류: ./nop-server.py 파일이 없거나 실행 가능한 파일이 아닙니다."
    exit
fi

# 필요하면 테스트 디렉터리를 생성합니다.
if [ ! -d ${PROXY_DIR} ]
then
    mkdir ${PROXY_DIR}
fi

if [ ! -d ${NOPROXY_DIR} ]
then
    mkdir ${NOPROXY_DIR}
fi

# 시간 초과 시 이해하기 쉬운 메시지를 출력하는 핸들러를 등록합니다.
trap 'echo "시간 초과: 서버가 예약된 포트를 점유할 때까지 기다렸지만 완료되지 않았습니다."; kill $$' ALRM

#####
# 기본 테스트
#
echo "*** 기본 테스트 ***"

# Tiny 웹 서버를 실행합니다.
tiny_port=$(free_port)
echo "tiny를 포트 ${tiny_port}에서 시작합니다."
cd ./tiny
./tiny ${tiny_port}   &> /dev/null  &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${tiny_port}"

# proxy를 실행합니다.
proxy_port=$(free_port)
echo "proxy를 포트 ${proxy_port}에서 시작합니다."
./proxy ${proxy_port}  &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${proxy_port}"


# Tiny에서 직접 가져온 파일과 프록시를 통해 가져온 파일을 비교해 테스트합니다.
numRun=0
numSucceeded=0
for file in ${BASIC_LIST}
do
    numRun=`expr $numRun + 1`
    echo "${numRun}: ${file}"
    clear_dirs

    # 프록시를 통해 파일을 가져옵니다.
    echo "   프록시를 사용해 ./tiny/${file} 파일을 ${PROXY_DIR}로 가져옵니다."
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"

    # Tiny에서 직접 파일을 가져옵니다.
    echo "   Tiny에서 직접 ./tiny/${file} 파일을 ${NOPROXY_DIR}로 가져옵니다."
    download_noproxy $NOPROXY_DIR ${file} "http://localhost:${tiny_port}/${file}"

    # 두 파일을 비교합니다.
    echo "   두 파일을 비교합니다."
    diff -q ${PROXY_DIR}/${file} ${NOPROXY_DIR}/${file} &> /dev/null
    if [ $? -eq 0 ]; then
        numSucceeded=`expr ${numSucceeded} + 1`
        echo "   성공: 두 파일이 동일합니다."
    else
        echo "   실패: 두 파일이 서로 다릅니다."
    fi
done

echo "tiny와 proxy를 종료합니다."
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

basicScore=`expr ${MAX_BASIC} \* ${numSucceeded} / ${numRun}`

echo "basicScore: $basicScore/${MAX_BASIC}"


######
# 동시성 테스트
#

echo ""
echo "*** 동시성 테스트 ***"

# Tiny 웹 서버를 실행합니다.
tiny_port=$(free_port)
echo "tiny를 포트 ${tiny_port}에서 시작합니다."
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${tiny_port}"

# proxy를 실행합니다.
proxy_port=$(free_port)
echo "proxy를 포트 ${proxy_port}에서 시작합니다."
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${proxy_port}"

# 요청에 응답하지 않는 특수한 차단용 nop-server를 실행합니다.
nop_port=$(free_port)
echo "차단용 NOP 서버를 포트 ${nop_port}에서 시작합니다."
./nop-server.py ${nop_port} &> /dev/null &
nop_pid=$!

# nop 서버가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${nop_port}"

# 프록시를 통해 차단용 nop-server에서 파일을 가져오도록 시도합니다.
clear_dirs
echo "차단용 nop-server에서 파일을 가져오도록 시도합니다."
download_proxy $PROXY_DIR "nop-file.txt" "http://localhost:${nop_port}/nop-file.txt" "http://localhost:${proxy_port}" &

# Tiny에서 직접 파일을 가져옵니다.
echo "./tiny/${FETCH_FILE} 파일을 Tiny에서 직접 ${NOPROXY_DIR}로 가져옵니다."
download_noproxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}"

# 프록시를 통해 파일을 가져옵니다.
echo "프록시를 사용해 ./tiny/${FETCH_FILE} 파일을 ${PROXY_DIR}로 가져옵니다."
download_proxy $PROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# 프록시를 통한 가져오기가 성공했는지 확인합니다.
echo "프록시를 통한 파일 가져오기가 성공했는지 확인합니다."
diff -q ${PROXY_DIR}/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE} &> /dev/null
if [ $? -eq 0 ]; then
    concurrencyScore=${MAX_CONCURRENCY}
    echo "성공: proxy를 통해 tiny/${FETCH_FILE} 파일을 가져올 수 있었습니다."
else
    concurrencyScore=0
    echo "실패: proxy를 통해 tiny/${FETCH_FILE} 파일을 가져오지 못했습니다."
fi

# 정리 작업
echo "tiny, proxy, nop-server를 종료합니다."
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null
kill $nop_pid 2> /dev/null
wait $nop_pid 2> /dev/null

echo "concurrencyScore: $concurrencyScore/${MAX_CONCURRENCY}"

#####
# 캐시 테스트
#
echo ""
echo "*** 캐시 테스트 ***"

# Tiny 웹 서버를 실행합니다.
tiny_port=$(free_port)
echo "tiny를 포트 ${tiny_port}에서 시작합니다."
cd ./tiny
./tiny ${tiny_port} &> /dev/null &
tiny_pid=$!
cd ${HOME_DIR}

# tiny가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${tiny_port}"

# proxy를 실행합니다.
proxy_port=$(free_port)
echo "proxy를 포트 ${proxy_port}에서 시작합니다."
./proxy ${proxy_port} &> /dev/null &
proxy_pid=$!

# proxy가 실제로 시작될 때까지 기다립니다.
wait_for_port_use "${proxy_port}"

# 프록시를 통해 Tiny에서 여러 파일을 가져옵니다.
clear_dirs
for file in ${CACHE_LIST}
do
    echo "프록시를 사용해 ./tiny/${file} 파일을 ${PROXY_DIR}로 가져옵니다."
    download_proxy $PROXY_DIR ${file} "http://localhost:${tiny_port}/${file}" "http://localhost:${proxy_port}"
done

# Tiny를 종료합니다.
echo "tiny를 종료합니다."
kill $tiny_pid 2> /dev/null
wait $tiny_pid 2> /dev/null

# 앞에서 가져온 파일 중 하나를 캐시에서 다시 가져와 봅니다.
echo "./tiny/${FETCH_FILE} 파일의 캐시 사본을 ${NOPROXY_DIR}로 가져옵니다."
download_proxy $NOPROXY_DIR ${FETCH_FILE} "http://localhost:${tiny_port}/${FETCH_FILE}" "http://localhost:${proxy_port}"

# tiny 디렉터리의 원본 파일과 비교해 프록시 가져오기가 성공했는지 확인합니다.
diff -q ./tiny/${FETCH_FILE} ${NOPROXY_DIR}/${FETCH_FILE}  &> /dev/null
if [ $? -eq 0 ]; then
    cacheScore=${MAX_CACHE}
    echo "성공: 캐시에서 tiny/${FETCH_FILE} 파일을 가져올 수 있었습니다."
else
    cacheScore=0
    echo "실패: 프록시 캐시에서 tiny/${FETCH_FILE} 파일을 가져오지 못했습니다."
fi

# proxy를 종료합니다.
echo "proxy를 종료합니다."
kill $proxy_pid 2> /dev/null
wait $proxy_pid 2> /dev/null

echo "cacheScore: $cacheScore/${MAX_CACHE}"

# 총점을 출력합니다.
totalScore=`expr ${basicScore} + ${cacheScore} + ${concurrencyScore}`
maxScore=`expr ${MAX_BASIC} + ${MAX_CACHE} + ${MAX_CONCURRENCY}`
echo ""
echo "totalScore: ${totalScore}/${maxScore}"
exit
