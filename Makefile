.PHONY: all test clean

all:
	cd src; make
test:
	cd src; make test

clean:
	rm exec/a.out
