FINAL_CFLAGS=-W -std=c++11 $(DEBUG) -pthread

OPTIMIZATION?=-O2
DEBUG=-g

SERVER_CC=$(CXX) $(FINAL_CFLAGS)

SUM_SERVER_NAME=server
SUM_CLIENT_NAME=client
SUM_SERVER_OBJ= main.o util.o log.o
SUM_CLIENT_OBJ= clientmain.o util.o log.o 

all: $(SUM_SERVER_NAME) $(SUM_CLIENT_NAME)
	@echo "hello!"
	@echo $(CXX)
	@echo $(SERVER_CC)

$(SUM_SERVER_NAME): $(SUM_SERVER_OBJ)
	$(SERVER_CC) -o $@ $^

$(SUM_CLIENT_NAME): $(SUM_CLIENT_OBJ)
	$(SERVER_CC) -o $@ $^

%.o: %.cpp
	$(SERVER_CC) -c $<

.PHONY : clean
clean:
	rm  -rf $(SUM_SERVER_OBJ) clientmain.o
