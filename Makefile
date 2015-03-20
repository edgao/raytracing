.PHONY: all test clean

all:
	cd src; make
test:
	cd src; make test
dev:
	cd src; make dev

clean:
	rm exec/a.out
