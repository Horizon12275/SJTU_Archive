
LINK.o = $(LINK.cc)
CXXFLAGS = -std=c++20 -Wall -g

all: correctness persistence test

correctness: kvstore.o correctness.o memtable.o sstable.o vlog.o

persistence: kvstore.o persistence.o memtable.o sstable.o vlog.o

test: kvstore.o test.o memtable.o sstable.o vlog.o

clean:
	-rm -f correctness persistence test *.o
