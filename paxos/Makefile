FINAL_CFLAGS=-W -std=c++11 $(DEBUG) 
DYNAMIC_INLUDE=-lpthread -lcrypto -lssl

OPTIMIZATION?=-O2
DEBUG=-g

PAXOS_CC=$(CXX) $(FINAL_CFLAGS) $(OPTIMIZATION)

SUM_PAXOS_NAME=cluster
SUM_PAXOS_OBJ=main.o

all : $(SUM_PAXOS_NAME)
	@echo "Hello!"

Makefile.dep:
	-$(PAXOS_CC) -MM *.cpp > Makefile.dep 2> /dev/null || true

-include Makefile.dep

 $(SUM_PAXOS_NAME):$(SUM_PAXOS_OBJ)
	$(PAXOS_CC) -o $@ $^ $(DYNAMIC_INLUDE)

%.o: %.cpp
	$(PAXOS_CC) -c $<

.PHONY : clean
clean:
	rm *.o $(SUM_PAXOS_NAME)