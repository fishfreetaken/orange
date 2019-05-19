
vpath %.h ../include/

CXXFLAGS = -g -I../include/
all: a b
.PHONY : all

a: anysort.o  main.o 
	$(CXX)  $(CXXFLAGS) $^ -o $@  -lpthread

b: clintmain.o
	$(CXX)  $(CXXFLAGS) $^ -o $@  -lpthread

%.o: %.c .make-prerequisites
	$(CXX) -c $<

.PHONY:clean

clean:
	rm *.o


