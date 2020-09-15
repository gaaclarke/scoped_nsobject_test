CC=clang++
CCFLAGS=-c -g -std=c++17
LDFLAGS=-framework Foundation

all: main_arc main_mrc
main_arc.o: main.mm scoped_nsobject.h
	$(CC) $(CCFLAGS) -fobjc-arc -o main_arc.o main.mm
main_mrc.o: main.mm scoped_nsobject.h
	$(CC) $(CCFLAGS) -fno-objc-arc -o main_mrc.o main.mm 
main_arc: main_arc.o scoped_nsobject.o
	$(CC) $(LDFLAGS) main_arc.o scoped_nsobject.o -o main_arc
main_mrc: main_mrc.o scoped_nsobject.o
	$(CC) $(LDFLAGS) main_mrc.o scoped_nsobject.o -o main_mrc
scoped_nsobject.o: scoped_nsobject.mm scoped_nsobject.h
	$(CC) $(CCFLAGS) -fno-objc-arc -o scoped_nsobject.o scoped_nsobject.mm 