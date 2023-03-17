
CC=g++
# CFLAG = -Wall -Werror -pedantic -g -std=c++1y
# CFLAG = -g -std=c++1y
CFLAG = -g -std=c++17

solver: MapColoringDPLL.cpp
	${CC} ${CFLAG} -o solver MapColoringDPLL.cpp

clean:
	rm -f solver