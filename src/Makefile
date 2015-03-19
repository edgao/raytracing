.PHONY: all test

all:
	g++ -I ../lib/ -std=c++11 Class.cpp main.cpp -o ../exec/main.out
test:
	g++ -g -I ../lib Class.cpp test.cpp -o ../exec/test.out

dev:
	g++ -I ../lib -std=c++11 Class.cpp main.cpp -o main.out -O2 -L/usr/X11R6/lib -lm -lpthread -lX11 -ljpeg
