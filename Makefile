CXX = g++
CFLAGS = -Wall -O2 -pipe -c
PACKAGE = `pkg-config --cflags gtk+-2.0`
LIBS = `pkg-config --libs gtk+-2.0` -lgmp -lmpfr -pthread
EXE = mandelbrot

SRCS = main.cpp mandelbrot.cpp color.cpp
OBJS = main.o mandelbrot.o color.o

.PHONY : main build clean

main : build       

build: $(OBJS)
	$(CXX) $(LIBS) $(OBJS) -pipe -o $(EXE)

%.o: %.cpp
	$(CXX) $(PACKAGE) $(CFLAGS) $< -o $@ 

clean: 
	rm $(OBJS) $(EXE) -f
